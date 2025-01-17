#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum Type { PIPE, SEMI, QUOTE, LITERAL, END, REDIRECT, CONTROL };

struct Token {
	Type type{Type::END};
	std::string value{"END"};

	bool operator==(const Token& other) const {
		return type == other.type && value == other.value;
	}

	friend std::ostream& operator<<(std::ostream& os, const Token& token) {
		os << token.type << " " << token.value;
		return os;
	}
};

#endif
