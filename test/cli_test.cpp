#include <gtest/gtest.h>

#include <cli.hpp>
#include <global_state.hpp>
#include <parser.hpp>
#include <text_input.hpp>
#include <text_output.hpp>

namespace coreutils::test {

class CLITest : public ::testing::Test {
 protected:
  void SetUp() override {
    parser = std::make_unique<Parser>();
    cli = std::make_unique<CLI>(*parser);

    IsExit = false;
  }

  void TearDown() override { IsExit = false; }

  std::unique_ptr<Parser> parser;
  std::unique_ptr<CLI> cli;
};

TEST_F(CLITest, RunCliSingleCommand) {
  TextOutput output;
  TextInput input("echo test");
  EXPECT_NO_THROW(cli->runCli(input, output));
  EXPECT_EQ(output.read(), "test\n");
}

TEST_F(CLITest, RunCliEnvVariables) {
  TextOutput output;
  TextInput input("a=123 echo ${a}");
  EXPECT_NO_THROW(cli->runCli(input, output));
  EXPECT_EQ(output.read(), "123\n");
}

}  // namespace coreutils::test
