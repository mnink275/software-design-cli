#include <gtest/gtest.h>

#include <cli.hpp>
#include <global_state.hpp>
#include <parser.hpp>
#include <text_input.hpp>
#include <text_output.hpp>

#include <filesystem>

namespace coreutils::test {

namespace {

class ScopedChdir {
 public:
  ScopedChdir() : initial_(std::filesystem::current_path()) {}
  ScopedChdir(const ScopedChdir&) = delete;
  ScopedChdir& operator=(const ScopedChdir&) = delete;
  ScopedChdir(ScopedChdir&&) = delete;
  ScopedChdir& operator=(ScopedChdir&&) = delete;
  ~ScopedChdir() { std::filesystem::current_path(initial_); }

 private:
  std::filesystem::path initial_;
};

}  // namespace

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
  TextInput input("a=123 sh -c 'echo ${a}'");
  EXPECT_NO_THROW(cli->runCli(input, output));
  EXPECT_EQ(output.read(), "123\n");
}

TEST_F(CLITest, RunCliCdAndPwd) {
  ScopedChdir guard;
  const auto target = std::filesystem::path(TEST_DATA_DIR);

  TextOutput output;
  TextInput input("cd " + target.string() + "\npwd\n");
  EXPECT_NO_THROW(cli->runCli(input, output));
  EXPECT_EQ(output.read(), target.string() + "\n");
}

TEST_F(CLITest, RunCliCdAffectsExternalCommand) {
  ScopedChdir guard;

  const auto target = std::filesystem::path(TEST_DATA_DIR);

  TextOutput output;
  TextInput input("cd " + target.string() + "\n/bin/pwd\n");
  EXPECT_NO_THROW(cli->runCli(input, output));
  EXPECT_EQ(output.read(), target.string() + "\n");
}

}  // namespace coreutils::test
