#ifndef SHELLERROR_H
#define SHELLERROR_H

#include <string>

enum ErrorType { UNCLOSED_QUOTE, SYNTAX_ERROR };

struct ShellError {
	ErrorType type;
	std::string message;

	bool operator==(const ShellError& other) const {
		return type == other.type && message == other.message;
	}
};

#endif
