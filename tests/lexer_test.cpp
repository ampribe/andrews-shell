#include <gtest/gtest.h>
#include <string>
#include <utility>
#include "lexer.h"
#include <token.h>

class LexerTest : public testing::Test {
protected:
	void SetUp() override {}
	LexerTest() {}
	void testLexer(std::string input, std::vector<std::pair<Type, std::string>> expected) {
		Lexer lexer(input);
		Token tok;
		for (const auto& [type, value] : expected) {
			tok = lexer.getToken();
			EXPECT_EQ(tok.type, type);
			EXPECT_EQ(tok.value, value);
		}
	}
};

TEST_F(LexerTest, EmptyInput) {
	std::string input = "";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::END, ""},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand1) {
	std::string input = "echo asdf";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::LITERAL, "echo"},
		{Type::LITERAL, "asdf"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand2) {
	std::string input = "echo asdf 123 \"%%%\"";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::LITERAL, "echo"},
		{Type::LITERAL, "asdf"},
		{Type::LITERAL, "123"},
		{Type::QUOTE, "%%%"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, SingleCommand3) {
	std::string input = "		asdf  123 \"%%%\"	";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::LITERAL, "asdf"},
		{Type::LITERAL, "123"},
		{Type::QUOTE, "%%%"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Pipeline1) {
	std::string input = "		asdf  123 | \"%%%\"	";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::LITERAL, "asdf"},
		{Type::LITERAL, "123"},
		{Type::PIPE, "|"},
		{Type::QUOTE, "%%%"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Pipeline2) {
	std::string input = "| |		|		asdf  123 | \"%%%\"	";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::PIPE, "|"},
		{Type::PIPE, "|"},
		{Type::PIPE, "|"},
		{Type::LITERAL, "asdf"},
		{Type::LITERAL, "123"},
		{Type::PIPE, "|"},
		{Type::QUOTE, "%%%"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, MultipleCommands) {
	std::string input = "| |;		|		asdf  123 | \"%%%\"	";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::PIPE, "|"},
		{Type::PIPE, "|"},
		{Type::SEMI, ";"},
		{Type::PIPE, "|"},
		{Type::LITERAL, "asdf"},
		{Type::LITERAL, "123"},
		{Type::PIPE, "|"},
		{Type::QUOTE, "%%%"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Redirection1) {
	std::string input = ">> <<";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::REDIRECT, ">>"},
		{Type::LITERAL, "<<"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

TEST_F(LexerTest, Redirection2) {
	std::string input = "2>&1 &> 2>> 1>&2 >";
	std::vector<std::pair<Type, std::string>> expected = {
		{Type::REDIRECT, "2>&1"},
		{Type::REDIRECT, "&>"},
		{Type::REDIRECT, "2>>"},
		{Type::REDIRECT, "1>&2"},
		{Type::REDIRECT, ">"},
		{Type::END, ""}
	};
	testLexer(input, expected);
}

