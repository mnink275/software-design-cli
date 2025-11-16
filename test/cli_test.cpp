#include <gtest/gtest.h>

#include <thread>

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

  auto run_cli_thread = std::thread{[this, &output]() {
    TextInput input("echo test");

    EXPECT_NO_THROW(cli->runCli(input, output));
  }};

  while (output.read().empty()) {
    // waiting for command execution
  }

  IsExit = true;

  run_cli_thread.join();

  EXPECT_EQ(output.read(), "test\n");
}

// TEST_F(CLITest, RunCliEnvVariables) {
//   TextOutput output;

//   auto run_cli_thread = std::thread{[this, &output]() {
//     TextInput input("VAR=erm sh -c 'echo ${VAR}'");

//     EXPECT_NO_THROW(cli->runCli(input, output));
//   }};

//   while (output.read().empty()) {
//     // waiting for command execution
//   }

//   IsExit = true;

//   run_cli_thread.join();

//   EXPECT_EQ(output.read(), "VAR=erm\n");
// }

}  // namespace coreutils::test
