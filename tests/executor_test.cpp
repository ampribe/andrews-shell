#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <variant>
#include "executor.h"
#include "lexer.h"
#include "token.h"
#include "shellerror.h"

class ExecutorTest : public testing::Test {
protected:
	void SetUp() override {}
	ExecutorTest() {}
	void testExecutor(std::string input, std::string expected) {
		Lexer lexer(input);
		Parser parser(lexer);
		auto sequence = parser.parse();

		Executor executor;
		std::stringstream buffer;

		std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
		executor.execute(sequence);
		std::cout << std::endl;
		std::cout.rdbuf(old);

		std::string output = buffer.str();

		EXPECT_EQ(output, expected);
	}
};

TEST_F(ExecutorTest, BasicInput) {
	std::string input = "echo blah";
	std::string expected = "blah\n";
	testExecutor(input, expected);
}
