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

enum RedirectType { IN, OUT, APPEND };

struct Redirect {
	RedirectType type;
	int fdFrom{1};
	int fdTo{0};
	std::string file{""};

	bool operator==(const Redirect& other) const {
		return type == other.type && fdFrom == other.fdFrom && fdTo == other.fdTo && file == other.file;
	}
	friend std::ostream& operator<<(std::ostream& os, const Redirect& redirect) {
		os << "{" << redirect.type << ", " << redirect.fdFrom << ", " << redirect.fdTo << ", " << redirect.file << "}";
		return os;
	}
};

struct Command {
	std::vector<std::string> args{};
	std::vector<Redirect> redirections{};
	bool background{false};
	
	bool operator==(const Command& other) const {
		return args == other.args && redirections == other.redirections && background == other.background;
	}
	friend std::ostream& operator<<(std::ostream& os, const Command& cmd) {
		os << "Command {\nArgs: [";
		for (const auto& arg : cmd.args) {
			os << arg << ", ";
		}
		os << "]\nRedirections[";
		for (const auto& redirection : cmd.redirections) {
			os << redirection << ", ";
		}
		os << "]\n}\n";
		return os;
	}
};

struct Pipeline {
	std::vector<Command> commands;

	bool operator==(const Pipeline& other) const {
		return commands == other.commands;
	}
	friend std::ostream& operator<<(std::ostream& os, const Pipeline& pipeline) {
		os << "\nPipeline{\n";
		for (const auto& cmd : pipeline.commands) {
			os << cmd;
		}
		os << "}";
		return os;
	}
};

inline std::ostream& operator<<(std::ostream& os, const std::variant<Pipeline, ShellError>& v) {
    std::visit([&os](const auto& val) { os << val; }, v);
    return os;
}

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
		if (isTokenType(Type::SEMI)) {
			getToken();
		}
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
				auto result = readRedirect();
				if (std::holds_alternative<ShellError>(result)) {
					advanceToCommandEnd();
					return std::get<ShellError>(token);
				}
				std::vector<Redirect> res = std::get<std::vector<Redirect>>(result);
				command.redirections.insert(command.redirections.end(), res.begin(), res.end());
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
	std::variant<std::vector<Redirect>, ShellError> readRedirect() {
		ShellError error = {ErrorType::SYNTAX_ERROR, "Error: Invalid redirection."};
		Token tok = std::get<Token>(token);
		getToken();
		if (tok.value == "<") {
			if (isArgument()) {
				auto res = std::vector<Redirect>{{.type = RedirectType::IN, .file = getArgument()}};
				getToken();
				return res;
			}
			return error;
		} else if (tok.value == ">") {
			if (isArgument()) {
				auto res = std::vector<Redirect>{{.type = RedirectType::OUT, .file = getArgument()}};
				getToken();
				return res;
			}
			return error;
		} else if (tok.value == ">>") {
			if (isArgument()) {
				auto res = std::vector<Redirect>{{.type = RedirectType::APPEND, .file = getArgument()}};
				getToken();
				return res;
			}
			return error;
		} else if (tok.value == "&>") {
			if (isArgument()) {
				auto res = std::vector<Redirect>{
					Redirect {
						.type = RedirectType::OUT,
						.fdFrom = 1,
						.file = getArgument()
					},
					Redirect {
						.type = RedirectType::OUT,
						.fdFrom = 2,
						.file = getArgument()
					},
				};
				getToken();
				return res;
			} 
			return error;
		} else if (tok.value == "&>>") {
			if (isArgument()) {
				auto res = std::vector<Redirect>{
					Redirect {
						.type = RedirectType::APPEND,
						.fdFrom = 1,
						.file = getArgument()
					},
					Redirect {
						.type = RedirectType::APPEND,
						.fdFrom = 2,
						.file = getArgument()
					},
				};
				getToken();
				return res;
			} 
			return error;
		}
		std::regex pattern;
		std::smatch matches;
		pattern.assign(R"((\d)>&(\d))");
		if (std::regex_search(tok.value, matches, pattern)) {
			return std::vector<Redirect> {
				Redirect {
					.type = RedirectType::OUT,
					.fdFrom = std::stoi(matches[1]),
					.fdTo = std::stoi(matches[2])
				}
			};
		}
		pattern.assign(R"((\d)>>)");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (isArgument()) {
				auto res = std::vector<Redirect>{
					{
						.type = RedirectType::APPEND,
						.fdFrom = std::stoi(matches[1]),
						.file = getArgument()
					}
				};
				getToken();
				return res;
			}
			return error;
		}
		pattern.assign(R"(>&(\d))");
		if (std::regex_search(tok.value, matches, pattern)) {
			return std::vector<Redirect> {
				Redirect {
					.type = RedirectType::OUT,
					.fdFrom = 1,
					.fdTo = std::stoi(matches[1])
				}
			};
		}
		pattern.assign(R"((\d)>)");
		if (std::regex_search(tok.value, matches, pattern)) {
			if (isArgument()) {
				auto res = std::vector<Redirect>{
					{
						.type = RedirectType::OUT,
						.fdFrom = std::stoi(matches[1]),
						.file = getArgument()
					}
				};
				getToken();
				return res;
			}
			return error;
		}
		return error;
	}
};
#endif
