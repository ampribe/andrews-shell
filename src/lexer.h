#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>
#include <optional>
#include <cctype>
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
		return lexLiteralOrSpecial();
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
	Type getSpecialType(std::string s) {
		if (s == "|") {
			return Type::PIPE;
		} else if (s == "&") {
			return Type::CONTROL;
		} else if (s == ";") {
			return Type::SEMI;
		} else {
			return Type::REDIRECT;
		}
	}
	Token lexLiteralOrSpecial() {
		std::string str;
		str.push_back(line[pos]);
		if (isSpecial(str)) {
			return lexSpecial();
		} else if (std::isdigit(line[pos])) {
			if (pos + 1 < line.length()) {
				str.push_back(line[pos+1]);
				if (isSpecial(str)) {
					return lexSpecial();
				}
			}
		} 
		return lexLiteral();
	}
	Token lexSpecial() {
		std::string str;
		str.push_back(line[pos]);
		pos++;
		while (pos < line.length()) {
			str.push_back(line[pos]);
			if (!isSpecial(str)) {
				str.pop_back();
				return Token {getSpecialType(str), std::move(str)};
			}
			pos++;
		}
		return Token {getSpecialType(str), std::move(str)};
	}
	Token lexLiteral() {
		std::string s = "";
		bool escape = false;
		while (pos < line.length() && !isspace(line[pos])) {
			if (line[pos] != '\\') {
				std::string str;
				str.push_back(line[pos]);
				if (!escape && isSpecial(str)) {
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

