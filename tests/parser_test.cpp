#include <gtest/gtest.h>
#include <string>
#include <variant>
#include "parser.h"
#include "lexer.h"
#include "token.h"
#include "shellerror.h"

class ParserTest : public testing::Test {
protected:
	void SetUp() override {}
	ParserTest() {}
	void testParser(std::string input, std::vector<std::variant<Pipeline, ShellError>> expected) {
		Lexer lexer(input);
		Parser parser(lexer);
		auto res = parser.parse();
		EXPECT_EQ(res, expected);
	}
};

TEST_F(ParserTest, BasicInput) {
	std::string input = "echo blah";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"echo", "blah"},
				}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, MultipleCommands) {
	std::string input = "echo blah; echo blah2";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"echo", "blah"},
				}
			}
		},
		Pipeline {
			.commands = {
				{
					.args = {"echo", "blah2"},
				}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, BasicError) {
	std::string input = "& blah";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		ShellError {ErrorType::SYNTAX_ERROR, "Error: \"&\" may only be used at the end of a command"}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, Pipeline1) {
	std::string input = "ls -la | cat";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"ls", "-la"},
				},
				{
					.args = {"cat"},
				}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, Pipeline2) {
	std::string input = "ls -la | cat | |&";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"ls", "-la"},
				},
				{
					.args = {"cat"},
				},
				{},
				{.background = true}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, Pipeline3) {
	std::string input = "ls -la | cat | | \"";
	std::vector<std::variant<Pipeline, ShellError>> expected = {};
	testParser(input, expected);
}

TEST_F(ParserTest, Redirect1) {
	std::string input = "ls -la > out.txt";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"ls", "-la"},
					.redirection = {
						.coutFile = "out.txt",
					}
				}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, Redirect2) {
	std::string input = "ls -la &> out.txt";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"ls", "-la"},
					.redirection = {
						.coutFile = "out.txt",
						.cerrFile = "out.txt"
					}
				}
			}
		}
	};
	testParser(input, expected);
}

TEST_F(ParserTest, Redirect3) {
	std::string input = "ls -la 2>&1 out.txt";
	std::vector<std::variant<Pipeline, ShellError>> expected = {
		Pipeline {
			.commands = {
				{
					.args = {"ls", "-la", "out.txt"},
					.redirection = {
						.cerrTo = 1,
					}
				}
			}
		}
	};
	testParser(input, expected);
}