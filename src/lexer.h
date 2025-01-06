#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <token.h>

class Lexer {
public:
	Lexer(std::string s) :line {s}, pos {0} {}
	Token getToken() {
		findToken();
		if (pos == line.length()) {
			return Token {Type::END, ""};
		}
		if (line[pos] == '"') {
			return lexQuote();
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
	Token lexQuote() {
		std::string value;
		pos++;
		while (pos < line.length() && line[pos] != '"') {
			value += line[pos];
			pos++;
		}
		if (pos == line.length()) {
			std::cout << "Error: Unclosed quote.";
			return Token {Type::END, ""};
		}
		pos++;
		return Token {Type::QUOTE, std::move(value)};
	}
	bool isRedirect(std::string s) {
		std::regex r(R"([&\d]?>>?(&\d)?|<)");
		return std::regex_match(s, r);
	}
	bool isSpecial(std::string s) {
		return s == "|" || s == "&" || s == ";" || isRedirect(s);
	}
	Token lexLiteral() {
		//handle cases wwhere command sequences aren't separated by spaces
		std::string s = "";
		bool escape = false;
		bool prevEscape = false;
		while (pos < line.length() && !isspace(line[pos])) {
			if (line[pos] != '\\') {
				s.push_back(line[pos]);
				escape = false;
			} else {
				escape = true;
				prevEscape = true;
			}
			pos++;
		}
		if (!escape && isSpecial(s)) {
			if (s == "|") {
				return Token {Type::PIPE, std::move(s)};
			}
			if (s == ";") {
				return Token {Type::SEMI, std::move(s)};
			}
			if (s == "&") {
				return Token {Type::CONTROL, std::move(s)};
			}
			if (isRedirect(s)) {
				return Token {Type::REDIRECT, std::move(s)};
			}
		}
		return Token {Type::LITERAL, std::move(s)};
	}
};

