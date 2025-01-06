#include <iostream>
#include <string>
#include <vector>

enum RedirectType { IN, OUT, APPEND };

struct Redirect {
	RedirectType type;
	int fdFrom{1};
	int fdTo{0};
	std::string file;
};

struct Command {
	std::vector<std::string> args;
	std::vector<Redirect> redirections;
};

struct Pipeline {
	std::vector<Command> commands;
};
/*
class Parser {
public:
	Parser(std::vector<Token>) : 
};
*/
