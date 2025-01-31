#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <regex>
#include "lexer.h"
#include "token.h"
#include "shellerror.h"
#include "pipeline.h"

class Parser {
public:
	Parser(Lexer lexer) : lexer {std::move(lexer)} {}
	std::vector<std::variant<Pipeline, ShellError>> parse() {
		std::vector<std::variant<Pipeline, ShellError>> result;
		while (!isTokenType(Type::END)) {
			result.push_back(readPipeline());
		}
		return result;
	}
	std::string getString() {
		return cachedResult;
	}
private:
	Lexer lexer;
	std::string cachedResult = "No result yet\n";
	std::variant<Token, ShellError> token = ShellError {ErrorType::SYNTAX_ERROR, "Error: Parser did not initialize current token"};
	void getToken() {
		token = lexer.getToken();
	}
	bool isTokenType(Type type) {
		if (auto tokenPtr = std::get_if<Token>(&token)) {
			return type == tokenPtr->type;
		}
		return false;
	}
	bool atPipelineEnd() {
		return isTokenType(Type::SEMI) || isTokenType(Type::END);
	}
	bool atCommandEnd() {
		return isTokenType(Type::SEMI) || isTokenType(Type::END) || isTokenType(Type::PIPE);
	}
	//Advance to first token of next pipeline (or end)
	void advanceToNewPipeline() {
		while (!isTokenType(Type::SEMI) && !isTokenType(Type::END)) {
			getToken();
		}
		getToken();
	}
	//Advance to last token of command (delimiter)
	void advanceToCommandEnd() {
		while (!isTokenType(Type::SEMI) && !isTokenType(Type::END) && !isTokenType(Type::PIPE)) {
			getToken();
		}
	}
	std::variant<Pipeline, ShellError> readPipeline() {
		Pipeline pipeline;
		std::variant<Command, ShellError> command;
		do {
			getToken();
			command = readCommand();
			if (std::holds_alternative<ShellError>(command)) {
				advanceToNewPipeline();
				return std::get<ShellError>(command);
			}
			pipeline.commands.push_back(std::get<Command>(command));
		} while (isTokenType(Type::PIPE));

		return pipeline;
	}
	//Advance to delimiter ending command
	//Assumes current token is start of command
	std::variant<Command, ShellError> readCommand() {
		Command command;
		while (!atCommandEnd()) {
			if (std::holds_alternative<ShellError>(token)) {
				advanceToCommandEnd();
				return std::get<ShellError>(token);
			}
			Token currentToken = std::get<Token>(token);
			if (isTokenType(Type::QUOTE) || isTokenType(Type::LITERAL)) {
				command.args.push_back(std::move(currentToken.value));
				getToken();
			} else if (isTokenType(Type::REDIRECT)) {
				auto result = readRedirect(command);
				if (result == 1) {
					advanceToCommandEnd();
					return ShellError {ErrorType::SYNTAX_ERROR, "Error: Invalid redirection."};
				}
			} else {
				command.background = true;
				getToken();
				if (!atCommandEnd()) {
					advanceToCommandEnd();
					return ShellError {ErrorType::SYNTAX_ERROR, "Error: \"&\" may only be used at the end of a command"};
				}
			}
		}
		return command;
	}
	bool isArgument() {
		return isTokenType(Type::LITERAL) || isTokenType(Type::QUOTE);
	}
	std::string getArgument() {
		Token tok = std::get<Token>(token);
		return std::move(tok.value);
	}
	int readRedirect(Command& cmd) {
		Token tok = std::get<Token>(token);
		getToken();
		if (tok.value == "<") {
			if (isArgument()) {
				cmd.redirection.cinFile = getArgument();
				getToken();
				return 0;
			}
			return 1;
		} else if (tok.value == ">") {
			if (isArgument()) {
				cmd.redirection.coutFile = getArgument();
				getToken();
				return 0;
			}
			return 1;
		} else if (tok.value == ">>") {
			if (isArgument()) {
				cmd.redirection.coutFile = getArgument();
				cmd.redirection.coutFileAppend = true;
				getToken();
				return 0;
			}
			return 1;
		} else if (tok.value == "&>") {
			if (isArgument()) {
				cmd.redirection.coutFile = getArgument();
				cmd.redirection.cerrFile = getArgument();
				getToken();
				return 0;
			}
			return 1;
		} else if (tok.value == "&>>") {
			if (isArgument()) {
				cmd.redirection.coutFile = getArgument();
				cmd.redirection.cerrFile = getArgument();
				cmd.redirection.coutFileAppend = true;
				cmd.redirection.cerrFileAppend = true;
				getToken();
				return 0;
			}
			return 1;
		}
		std::regex pattern;
		std::smatch matches;
		pattern.assign(R"((\d)>&(\d))");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (std::stoi(matches[1]) == 1) {
				cmd.redirection.coutTo = std::stoi(matches[2]);
			} else if (std::stoi(matches[1]) == 2) {
				cmd.redirection.cerrTo = std::stoi(matches[2]);
			} else {
				return 1;
			}
			return 0;
		}
		pattern.assign(R"((\d)>>)");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (isArgument()) {
				if (std::stoi(matches[1]) == 1) {
					cmd.redirection.coutFile = getArgument();
					cmd.redirection.coutFileAppend = true;
				} else if (std::stoi(matches[1]) == 2) {
					cmd.redirection.cerrFile = getArgument();
					cmd.redirection.cerrFileAppend = true;
				} else {
					return 1;
				}
				getToken();
				return 0;
			}
			return 1;
		}
		pattern.assign(R"(>&(\d))");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (std::stoi(matches[1]) == 2) {
				cmd.redirection.coutTo = 2;
				return 0;
			} else {
				return 1;
			}
		}
		pattern.assign(R"((\d)>)");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (isArgument()) {
				if (std::stoi(matches[1]) == 1) {
					cmd.redirection.coutFile = getArgument();
				} else if (std::stoi(matches[1]) == 2) {
					cmd.redirection.cerrFile = getArgument();
				} else {
					return 1;
				}
				getToken();
				return 0;
			}
		}
		return 1;
	}
};
#endif
