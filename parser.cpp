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
#include <set>

enum Type { special, quote, literal, eof};
// operators: |, [n]<, [n]>>, [n]>, ;
//

struct Token {
	Type type;
	std::string* value;
};

class LineParser {
public:
	LineParser(std::string s) :line { s }, pos { 0 } { }
	int getToken(Token** ptr) {
		while (pos < line.length() && isspace(line[pos])) {
			pos++;
		}
		if (pos == line.length()) {
			*ptr = new Token {Type::eof, nullptr};
			return 0;
		}
		if (line[pos] == '"') {
			return parseQuote(ptr);
		}
		return parseLiteral(ptr);
	}
private:
	std::string line;
	size_t pos;
	int parseQuote(Token** ptr) {
		int closeLocation = line.find('"', pos+1);
		if (closeLocation < line.length()) {
			*ptr = new Token { Type::quote, new std::string{line.substr(pos+1, closeLocation-pos-1)} };
			pos = closeLocation+1;
			return 0;
		}
		return 1;
	}
	int parseLiteral(Token** ptr) {
		std::set<std::string> special = {"|", ">>", "<", ">", ";"};
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
		std::string* sptr = new std::string{ s };
		if (!escape && special.find(*sptr) != special.end()) {
			*ptr = new Token { Type::special, sptr };
		} else {
			*ptr = new Token { Type::literal, sptr };
		}
		return 0;
	}
};

int main(int argc, char* argv[]) {
	std::string line;
	std::getline(std::cin, line);
	LineParser parser(line);
	Token* t;
	parser.getToken(&t);
	while (t->type != Type::eof) {
		std::cout << "Token: " << *t->value << " Type: " << t->type << "\n";
		parser.getToken(&t);
	}
	std::cout << "finished";
	return 0;
}
