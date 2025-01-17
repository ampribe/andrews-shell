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
	//Do we just want to pass by reference instead?
	void executePipeline(Pipeline pipeline) {
		for (const auto& cmd : pipeline.commands) {
			executeCommand(cmd);
		}
	}
	void executeCommand(const Command& cmd) {
		if (!cmd.args.empty()) {
			if (cmd.args[0] == "exit") {
				exit(0);
			} else if (cmd.args[0] == "cd") {
				executeCd(cmd);
			} else {
				callCommand(cmd);
			}
		}
	}
	char** convertArgs(std::vector<std::string> vec) {
		char** args = new char*[vec.size() + 1];
		for (size_t i = 0; i < vec.size(); i++) {
			args[i] = new char[vec[i].length() + 1];
			strcpy(args[i], vec[i].c_str());
		}
		return args;
	}
	void callCommand(const Command& cmd) {
		int status;
		pid_t pid = fork();
		char** args = convertArgs(cmd.args);
		if (pid == 0) {
			execvp(args[0], args);
		} else {
			wait(&status);
		}
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
