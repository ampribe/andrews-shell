#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <optional>
#include <cctype>
#include <variant>
#include <filesystem>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "shellerror.h"
#include "pipeline.h"

class Executor {
public:
	Executor() {}
	void execute(std::vector<std::variant<Pipeline, ShellError>> sequence) {
		for (const auto& item : sequence) {
			if (auto ptr = std::get_if<ShellError>(&item)) {
				std::cout << ptr->message << std::endl;
			} else {
				auto pipeline = std::get<Pipeline>(item);
				executePipeline(pipeline);
			}
		}
	}
private:
	void executePipeline(const Pipeline& pipeline) {
		const size_t numCommands = pipeline.commands.size();
		int prevPipeFd[2] = {-1, -1};
		std::vector<pid_t> pids;

		for (size_t i = 0; i < numCommands; i++) {
			if (pipeline.commands[i].args[0] == "exit") {
					if (prevPipeFd[0] != -1) close(prevPipeFd[0]);
					if (prevPipeFd[1] != -1) close(prevPipeFd[1]);

					for (pid_t pid : pids) {
						int status;
						waitpid(pid, &status, 0);
				}
				exit(0);
			} else if (pipeline.commands[i].args[0] == "cd") {
				executeCd(pipeline.commands[i]);
				continue;
			}
			bool pipeIn = i > 0;
			bool pipeOut = i < numCommands - 1;
			int currPipeFd[2] = {-1, -1};
			if (pipeOut) {
				if (pipe(currPipeFd) == -1) {
					throw std::runtime_error("Failed to create pipe");
				}
			}

			pid_t pid = fork();
			if (pid < 0) {
				throw std::runtime_error("Fork failed");
			} else if (pid == 0) {
				if (pipeIn) {
					dup2(prevPipeFd[0], STDIN_FILENO);
				}
				if (pipeOut) {
					dup2(currPipeFd[1], STDOUT_FILENO);
				}
				if (prevPipeFd[0] != -1) close(prevPipeFd[0]);
				if (prevPipeFd[1] != -1) close(prevPipeFd[1]);
				if (currPipeFd[0] != -1) close(currPipeFd[0]);
				if (currPipeFd[1] != -1) close(currPipeFd[1]);
				callCommand(pipeline.commands[i], pipeIn, pipeOut);

				exit(EXIT_FAILURE);
			} else {
				pids.push_back(pid);

				if (pipeIn) {
					close(prevPipeFd[0]);
					close(prevPipeFd[1]);
				}
				prevPipeFd[0] = currPipeFd[0];
				prevPipeFd[1] = currPipeFd[1];
			}
		}

		if (prevPipeFd[0] != -1) close(prevPipeFd[0]);
		if (prevPipeFd[1] != -1) close(prevPipeFd[1]);

		for (pid_t pid : pids) {
			int status;
			waitpid(pid, &status, 0);
		}
	}
	char** convertArgs(std::vector<std::string> vec) {
		char** args = new char*[vec.size() + 1];
		for (size_t i = 0; i < vec.size(); i++) {
			args[i] = new char[vec[i].length() + 1];
			strcpy(args[i], vec[i].c_str());
		}
		args[vec.size()] = nullptr;
		return args;
	}
	void callCommand(const Command& cmd, bool pipeIn, bool pipeOut) {
		int coutfd = -1;
		int cinfd = -1;
		int cerrfd = -1;
		int status;

		if (cmd.redirection.coutFile != "" && !pipeIn) {
			coutfd = open(cmd.redirection.coutFile.c_str(), O_WRONLY | O_CREAT | (cmd.redirection.coutFileAppend ? O_APPEND : O_TRUNC), 0644);
		}
		if (cmd.redirection.cerrFile != "") {
			cerrfd = open(cmd.redirection.cerrFile.c_str(), O_WRONLY | O_CREAT | (cmd.redirection.cerrFileAppend ? O_APPEND : O_TRUNC), 0644);
		}
		if (cmd.redirection.cinFile != "" && !pipeOut) {
			cinfd = open(cmd.redirection.cinFile.c_str(), O_RDONLY);
		}
		
		pid_t pid = fork();
		char** args = convertArgs(cmd.args);
		if (pid == 0) {
			if (cmd.background) {
				setsid();
			}
			if (coutfd != -1) {
				dup2(coutfd, STDOUT_FILENO);
			}
			if (cerrfd != -1) {
				dup2(cerrfd, STDERR_FILENO);
			}
			if (cinfd != -1) {
				dup2(cinfd, STDIN_FILENO);
			}
			execvp(args[0], args);
		} else {
			// weird bug here
			if (cmd.background) {
				std::cout << "[" << pid << "] " << args[0] << std::endl;
			} else {
				wait(&status);
			}
			if (coutfd != -1) {
				close(coutfd);
			}
			if (cerrfd != -1) {
				close(cerrfd);
			}
			if (cinfd != -1) {
				close(cinfd);
			}

		}
		for (size_t i = 0; i < cmd.args.size(); i++) {
			delete[] args[i];
		}
		delete[] args;
	}
	void executeCd(const Command& cmd) {
		if (cmd.args.size() == 1) {
			std::filesystem::current_path(std::getenv("HOME"));
		} else if (cmd.args.size() == 2) {
			std::filesystem::path newpath = cmd.args[1];
			if (std::filesystem::is_directory(newpath)) {
				std::filesystem::current_path(newpath);
			} else {
				std::cout << "Error: Invalid directory";
			}
		} else {
			std::cout << "Error: Too many arguments.";
		}
	}
};
#endif
