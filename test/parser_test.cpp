#include <gtest/gtest.h>

#include <cstdlib>

#include <parser.hpp>

namespace coreutils::test {

class ParserTest : public ::testing::Test {
 protected:
  void SetUp() override {
    setenv("TEST_VAR", "test_value", 1);
    setenv("MY_VAR", "my_value", 1);
    setenv("FILE", "data.txt", 1);
    setenv("A", "AAA", 1);
    setenv("B", "BBB", 1);
    setenv("VAR", "value", 1);
  }

  void TearDown() override {
    unsetenv("TEST_VAR");
    unsetenv("MY_VAR");
    unsetenv("FILE");
    unsetenv("A");
    unsetenv("B");
    unsetenv("VAR");
  }

  Parser parser;
};

TEST_F(ParserTest, Basic) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo hello world");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "hello");
  EXPECT_EQ(tokens[2], "world");
}

TEST_F(ParserTest, EmptyInput) {
  Parser parser;
  auto tokens = parser.parseToTokens("");

  EXPECT_TRUE(tokens.empty());
}

TEST_F(ParserTest, EmptyQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo ''");

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0], "echo");
}

TEST_F(ParserTest, MultipleSpaces) {
  Parser parser;
  auto tokens = parser.parseToTokens("   cat    file.txt   ");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "cat");
  EXPECT_EQ(tokens[1], "file.txt");
}

TEST_F(ParserTest, OnlySpaces) {
  Parser parser;
  auto tokens = parser.parseToTokens("     ");

  EXPECT_TRUE(tokens.empty());
}

TEST_F(ParserTest, SingleQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo 'hello world'");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "hello world");
}

TEST_F(ParserTest, DoubleQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo \"hello world\"");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "hello world");
}

TEST_F(ParserTest, MixedQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo 'single' \"double\" normal");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "single");
  EXPECT_EQ(tokens[2], "double");
  EXPECT_EQ(tokens[3], "normal");
}

TEST_F(ParserTest, EnvironmentVariable) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo $TEST_VAR");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "test_value");
}

TEST_F(ParserTest, EnvironmentVariableWithBraces) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo ${MY_VAR}");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "my_value");
}

TEST_F(ParserTest, UndefinedVariable) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo $UNDEFINED_VAR_12345");

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0], "echo");
}

TEST_F(ParserTest, VariableInDoubleQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens(R"(echo "test $VAR end")");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "test value end");
}

// Будем игнорировать раскрытие в части 2
TEST_F(ParserTest, VariableInSingleQuotes) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo '$VAR'");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "value");
}

TEST_F(ParserTest, ComplexCommand) {
  auto tokens = parser.parseToTokens(
      R"(cat $FILE 'long name.txt' "another file" && echo "hello" | wc -l)");

  ASSERT_EQ(tokens.size(), 10);
  EXPECT_EQ(tokens[0], "cat");
  EXPECT_EQ(tokens[1], "data.txt");
  EXPECT_EQ(tokens[2], "long name.txt");
  EXPECT_EQ(tokens[3], "another file");
  EXPECT_EQ(tokens[4], "&&");
  EXPECT_EQ(tokens[5], "echo");
  EXPECT_EQ(tokens[6], "hello");
  EXPECT_EQ(tokens[7], "|");
  EXPECT_EQ(tokens[8], "wc");
  EXPECT_EQ(tokens[9], "-l");
}

TEST_F(ParserTest, MultipleVariables) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo $A $B");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "AAA");
  EXPECT_EQ(tokens[2], "BBB");
}

TEST_F(ParserTest, MixedMultipleVariables) {
  Parser parser;
  auto tokens = parser.parseToTokens("echo $A$B");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "AAABBB");
}

TEST_F(ParserTest, JustSetVariable) {
  Parser parser;
  auto tokens = parser.parseToTokens("a=erm");

  ASSERT_TRUE(tokens.empty());
}

TEST_F(ParserTest, ParseVariables) {
  Parser parser;
  std::ignore = parser.parseToTokens("ERM_VAR=erm a=123");
  auto tokens = parser.parseToTokens("echo ${ERM_VAR}${a}");

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0], "echo");
  EXPECT_EQ(tokens[1], "erm123");
}

TEST_F(ParserTest, SetVariablesBeforeCommand) {
  Parser parser;
  auto tokens = parser.parseToTokens("a=123 sh -c 'echo ${a}'");

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[0], "sh");
  EXPECT_EQ(tokens[1], "-c");
  EXPECT_EQ(tokens[2], "echo 123");
}

}  // namespace coreutils::test
