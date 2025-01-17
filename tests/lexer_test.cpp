#include <gtest/gtest.h>
#include <string>
#include <variant>
#include "lexer.h"
#include "token.h"
#include "shellerror.h"

class LexerTest : public testing::Test {
protected:
	void SetUp() override {}
	LexerTest() {}
	void testLexer(std::string input, std::vector<std::variant<Token, ShellError>> expected) {
		Lexer lexer(input);
		std::variant<Token, ShellError> tok;
		for (const auto& value: expected) {
			tok = lexer.getToken();
			EXPECT_EQ(tok, value);
		}
	}
};

TEST_F(LexerTest, EmptyInput) {
	std::string input = "";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::END, "END"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand1) {
	std::string input = "echo asdf";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::LITERAL, "echo"},
		Token {Type::LITERAL, "asdf"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand2) {
	std::string input = "echo asdf 123 \"%%%\"";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::LITERAL, "echo"},
		Token {Type::LITERAL, "asdf"},
		Token {Type::LITERAL, "123"},
		Token {Type::QUOTE, "%%%"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand3) {
	std::string input = "		asdf  123 \"%%%\"	";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::LITERAL, "asdf"},
		Token {Type::LITERAL, "123"},
		Token {Type::QUOTE, "%%%"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Pipeline1) {
	std::string input = "		asdf  123 | \"%%%\"	";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::LITERAL, "asdf"},
		Token {Type::LITERAL, "123"},
		Token {Type::PIPE, "|"},
		Token {Type::QUOTE, "%%%"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Pipeline2) {
	std::string input = "| |		|		asdf  123 | \"%%%\"	";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::PIPE, "|"},
		Token {Type::PIPE, "|"},
		Token {Type::PIPE, "|"},
		Token {Type::LITERAL, "asdf"},
		Token {Type::LITERAL, "123"},
		Token {Type::PIPE, "|"},
		Token {Type::QUOTE, "%%%"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, MultipleCommands) {
	std::string input = "| |;		|		asdf  123 | \"%%%\"	";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::PIPE, "|"},
		Token {Type::PIPE, "|"},
		Token {Type::SEMI, ";"},
		Token {Type::PIPE, "|"},
		Token {Type::LITERAL, "asdf"},
		Token {Type::LITERAL, "123"},
		Token {Type::PIPE, "|"},
		Token {Type::QUOTE, "%%%"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Redirection1) {
	std::string input = ">> \\<\\<";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::REDIRECT, ">>"},
		Token {Type::LITERAL, "<<"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Redirection2) {
	std::string input = "2>&1 &> 2>> 1>&2 >";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::REDIRECT, "2>&1"},
		Token {Type::REDIRECT, "&>"},
		Token {Type::REDIRECT, "2>>"},
		Token {Type::REDIRECT, "1>&2"},
		Token {Type::REDIRECT, ">"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Redirection3) {
	std::string input = "2>&1 <<<< &> 2>> 1>&2 >";
	std::vector<std::variant<Token, ShellError>> expected = {
		Token {Type::REDIRECT, "2>&1"},
		Token {Type::REDIRECT, "<"},
		Token {Type::REDIRECT, "<"},
		Token {Type::REDIRECT, "<"},
		Token {Type::REDIRECT, "<"},
		Token {Type::REDIRECT, "&>"},
		Token {Type::REDIRECT, "2>>"},
		Token {Type::REDIRECT, "1>&2"},
		Token {Type::REDIRECT, ">"},
		Token {Type::END, "END"}
	};
	testLexer(input, expected);
}

