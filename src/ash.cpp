/*
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <cctype>
#include <fcntl.h>
#include <unistd.h>

struct Command {
	std::vector<std::string> args;
	char* stderrRedirect = nullptr;
	char* stdinRedirect = nullptr;
	char* stdoutRedirect = nullptr;
	bool stderrRedirectAppend = false;
	bool stdoutRedirectAppend = false;
	bool pipeIn = false;
	bool pipeOut = false;
	bool background = false;
};

void tokenize(const std::string& line) {
	int start = 0;
	int current = 0;
	std::vector<Command> commands;
	Command cmd;
	bool escape = false;
	bool quotes = false;
	//bool variable = false;
	//bool command = false;
	// Implement variable expansion and command substitution later
	for (int current = 0; current < line.length(); current++) {
		if (isspace(line[current])) {
			if (
		}
	}
}

void callCommand(char* args[]) {
	int status;
	pid_t pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
	} else {
		wait(&status);
	}
}

char** convertArgs(std::vector<std::string> vec, int* elems) {
	*elems = vec.size();
	char** args = new char*[vec.size() + 1];
	for (size_t i = 0; i < vec.size(); i++) {
		args[i] = new char[vec[i].length() + 1];
		strcpy(args[i], vec[i].c_str());
	}
	return args;
}

std::vector<std::string> getArgs(const std::string& command) {
	std:: regex re("[^\\s]+");
	auto begin = std::sregex_iterator(command.begin(), command.end(), re);
	auto end = std::sregex_iterator();
	std::vector<std::string> args;
	for (std::sregex_iterator i = begin; i != end; ++i) {
		std::smatch match = *i;
		std::string arg = match.str();
		args.push_back(arg);
	}
	return args;
}

void executecd(char** args, int elems) {
	if (elems == 1) {
		std::filesystem::current_path(std::getenv("HOME"));
	} else if (elems == 2) {
		std::filesystem::path newpath = std::string(args[1]);
		if (std::filesystem::is_directory(newpath)) {
			std::filesystem::current_path(newpath);
		} else {
			std::cout << "Error: Invalid directory";
		}
	} else {
		std::cout << "Error: Too many arguments.";
	}
}

void executeCommand(const std::string& str) {
	int elems;
	char** args = convertArgs(getArgs(str), &elems);
	if (strcmp(args[0], "exit") == 0) {
		exit(0);
	} else if (strcmp(args[0], "cd") == 0) {
		executecd(args, elems);
	} else {
		callCommand(args);
	}
}

void executeLine(const std::string& str) {
	size_t prev = 0;
	size_t curr = 0;
	while ((curr = str.find(';', prev)) != std::string::npos) {
		std::string command = str.substr(prev, curr-prev);
		executeCommand(command);
		prev = curr + 1;
	}
	std::string command = str.substr(prev);
	executeCommand(command);
}

void runInteractiveMode() {
	std::string line;
	while (true) {
		std::cout << "ash> ";
		std::getline(std::cin, line);
		executeLine(line);
	}
}

void runBatchMode(char* filename) {
	std::string line;
	std::ifstream batch(filename);
	while (std::getline(batch, line)) {
		executeLine(line);
	}
	exit(0);
}

int main(int argc, char* argv[]) {
	try {
		if (argc > 2) {
			throw std::invalid_argument("Too many arguments");
		} else if (argc == 2) {
			runBatchMode(argv[1]);
		} else {
			runInteractiveMode();
		}
	} catch (const std::invalid_argument& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}
*/
