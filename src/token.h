#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum Type { PIPE, SEMI, QUOTE, LITERAL, END, REDIRECT, CONTROL };

struct Token {
	Type type;
	std::string value;
};

#endif
