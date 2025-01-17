#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <cctype>
#include <variant>
#include "token.h"
#include "shellerror.h"

class Lexer {
public:
	Lexer(std::string s) :line {s}, pos {0} {}
	std::variant<Token, ShellError> getToken() {
		findToken();
		if (pos == line.length()) {
			return Token {};
		}
		if (line[pos] == '|') {
			pos++;
			return Token {Type::PIPE, "|"};
		}
		if (line[pos] == ';') {
			pos++;
			return Token {Type::SEMI, ";"};
		}
		if (line[pos] == '&' && (pos + 1 == line.length() || line[pos + 1] != '>')) {
			pos++;
			return Token {Type::CONTROL, "&"};
		}
		if (line[pos] == '"') {
			return lexQuote();
		}
		std::optional<Token> tok = lexRedirect();
		if (tok.has_value()) {
			return tok.value();
		}
		return lexLiteral();
	}
private:
	std::string line;
	size_t pos;
	void findToken() {
		while (pos < line.length() && isspace(line[pos])) {
			pos++;
		}
	}
	std::variant<Token, ShellError> lexQuote() {
		std::string value;
		pos++;
		while (pos < line.length() && line[pos] != '"') {
			value += line[pos];
			pos++;
		}
		if (pos == line.length()) {
			return ShellError {ErrorType::UNCLOSED_QUOTE, "Error: Unclosed Quote"};
		}
		pos++;
		return Token {Type::QUOTE, std::move(value)};
	}
	// If current position points to redirect, greedy reads redirect and updates position
	// Redirect in form: 1) >, <, 2) \d>, >>, &>, 3) \d>>, &>>, >&\d, 4) \d>&\d
	std::optional<Token> lexRedirect() {
		std::regex pattern;
		std::string s;
		if (pos + 3 < line.length()) {
			pattern.assign(R"(\d>&\d)");
			s = line.substr(pos, 4);
			if (std::regex_match(s, pattern)) {
				pos += 4;
				return Token {Type::REDIRECT, std::move(s)};
			}
		}
		if (pos + 2 < line.length()) {
			pattern.assign(R"(([\d&]>>)|(>&\d))");
			s = line.substr(pos, 3);
			if (std::regex_match(s, pattern)) {
				pos += 3;
				return Token {Type::REDIRECT, std::move(s)};
			}
		}
		if (pos + 1 < line.length()) {
			pattern.assign(R"([\d>&]>)");
			s = line.substr(pos, 2);
			if (std::regex_match(s, pattern)) {
				pos += 2;
				return Token {Type::REDIRECT, std::move(s)};
			}
		}
		if (line[pos] == '>') {
			pos++;
			return Token {Type::REDIRECT, ">"};
		}
		if (line[pos] == '<') {
			pos++;
			return Token {Type::REDIRECT, "<"};
		}
		return std::nullopt;
	}
	bool isSpecial(char c) {
		return c == '|' || c == '&' || c == ';' || c == '<' || c == '>';
	}
	Token lexLiteral() {
		std::string s = "";
		bool escape = false;
		while (pos < line.length() && !isspace(line[pos])) {
			if (line[pos] != '\\') {
				if (!escape && isSpecial(line[pos])) {
					break;
				}
				s.push_back(line[pos]);
				escape = false;
			} else {
				escape = true;
			}
			pos++;
		}
		return Token {Type::LITERAL, std::move(s)};
	}
};
#endif
