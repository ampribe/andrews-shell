#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <regex>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>

void callCommand(char* args[]) {
	int status;
	pid_t pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
	} else {
		wait(&status);
	}
}

void executeCommand(const std::string& str) {
	std:: regex re("[^\\s]+");
	auto begin = std::sregex_iterator(str.begin(), str.end(), re);
	auto end = std::sregex_iterator();
	int elems = std::distance(begin, end);
	char* args[elems + 1];
	args[elems] = nullptr;
	int index = 0;
	for (std::sregex_iterator i = begin; i != end; ++i, ++index) {
		std::smatch match = *i;
		std::string arg = match.str();
		args[index] = new char[arg.length() + 1];
		std::strcpy(args[index], arg.c_str());
	}
	if (strcmp(args[0], "exit") == 0) {
		exit(0);
	} else if (strcmp(args[0], "cd") == 0) {
		if (elems == 1) {
			std::filesystem::current_path(std::getenv("HOME"));
		}
		if (elems == 2) {
			std::filesystem::path newpath = std::string(args[1]);
			if (std::filesystem::is_directory(newpath)) {
				std::filesystem::current_path(newpath);
			} else {
				std::cout << "Error: Invalid directory";
			}
		} else {
			std::cout << "Error: Too many arguments.";
		}
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

void runBatchMode() {
	std::cout << "Running batch mode...\n";
}

int main(int argc, char* argv[]) {
	try {
		if (argc > 2) {
			throw std::invalid_argument("Too many arguments");
		} else if (argc == 2) {
			runBatchMode();
		} else {
			runInteractiveMode();
		}
	} catch (const std::invalid_argument& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}
	return 0;
}
