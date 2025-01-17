#include <iostream>
#include <fstream>
#include "lexer.h"
#include "parser.h"
#include "executor.h"

void executeLine(std::string line) {
	auto executor = Executor();
	Lexer lexer(line);
	Parser parser(lexer);
	auto result = parser.parse();
	executor.execute(result);
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
