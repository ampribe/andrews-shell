#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include "shellerror.h"

enum RedirectType { IN, OUT, APPEND };

struct Redirect {
	int coutTo{1};
    int cerrTo{2};
    std::string coutFile {""};
    bool coutFileAppend {false};
    std::string cerrFile {""};
    bool cerrFileAppend {false};
    std::string cinFile {""};

	bool operator==(const Redirect& other) const {
		return coutTo == other.coutTo && cerrTo == other.cerrTo && coutFile == other.coutFile && cerrFile == other.cerrFile && cinFile == other.cinFile;
	}
friend std::ostream& operator<<(std::ostream& os, const Redirect& redirect) {
	os << "Redirect {\n"
	   << "  coutTo: " << redirect.coutTo << "\n"
	   << "  cerrTo: " << redirect.cerrTo << "\n"
	   << "  coutFile: " << redirect.coutFile << "\n"
	   << "  coutFileAppend: " << (redirect.coutFileAppend ? "true" : "false") << "\n"
	   << "  cerrFile: " << redirect.cerrFile << "\n"
	   << "  cerrFileAppend: " << (redirect.cerrFileAppend ? "true" : "false") << "\n"
	   << "  cinFile: " << redirect.cinFile << "\n"
	   << "}";
	return os;
}
};

struct Command {
	std::vector<std::string> args{};
	Redirect redirection{};
	bool background{false};
	
	bool operator==(const Command& other) const {
		return args == other.args && redirection == other.redirection && background == other.background;
	}
	friend std::ostream& operator<<(std::ostream& os, const Command& cmd) {
		os << "Command {\nArgs: [";
		for (const auto& arg : cmd.args) {
			os << arg << ", ";
		}
		os << "]\nRedirections[";
		os << cmd.redirection;
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
#endif
