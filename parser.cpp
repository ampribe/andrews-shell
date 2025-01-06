#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>
#include <set>
#include <optional>
#include <regex>

enum Type { special, quote, literal, eof};
// operators: |, [n]<, [n]>>, [n]>, ;
//

struct Token {
	Type type;
	std::string value;
};

class LineParser {
public:
	LineParser(std::string s) :line {s}, pos {0} {}
	std::optional<Token> getToken() {
		findToken();
		if (pos == line.length()) {
			return Token {Type::eof, ""};
		}
		if (line[pos] == '"') {
			return parseQuote();
		}
		return parseLiteral();
	}
private:
	std::string line;
	size_t pos;
	void findToken() {
		while (pos < line.length() && isspace(line[pos])) {
			pos++;
		}
	}
	std::optional<Token> parseQuote() {
		std::string value;
		pos++;
		while (pos < line.length() && line[pos] != '"') {
			value += line[pos];
			pos++;
		}
		if (pos == line.length()) {
			std::cout << "Error: Unclosed quote.";
			return std::nullopt;
		}
		pos++;
		return Token {Type::quote, std::move(value)};
	}
	bool isRedirect(std::string s) {
		std::regex r(R"([&\d]?>>?(&\d)?|<)");
		return std::regex_match(s, r);
	}
	std::optional<Token> parseLiteral() {
		std::set<std::string> special = {"|", ";"};
		std::string s = "";
		bool escape = false;
		while (pos < line.length() && !isspace(line[pos])) {
			if (line[pos] != '\\') {
				s.push_back(line[pos]);
			} else {
				escape = true;
			}
			pos++;
		}
		if (!escape && (s == "|" || s == ";" || isRedirect(s))) {
			return Token {Type::special, std::move(s)};
		}
		return Token {Type::literal, std::move(s)};
	}
};

int main(int argc, char* argv[]) {
	std::string line;
	std::getline(std::cin, line);
	LineParser parser(line);
	Token t = parser.getToken().value_or(Token{Type::eof, ""});
	while (t.type != Type::eof) {
		std::cout << "Token: " << t.value << " Type: " << t.type << "\n";
		t = parser.getToken().value_or(Token {Type::eof, ""});
	}
	std::cout << "finished\n";
	return 0;
}
