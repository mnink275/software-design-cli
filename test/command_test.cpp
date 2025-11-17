#include <gtest/gtest.h>

#include <cat_command.hpp>
#include <dummy_input.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <global_state.hpp>
#include <pwd_command.hpp>
#include <text_output.hpp>
#include <wc_command.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace coreutils::test {

namespace {

std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open()) {
    throw std::runtime_error("Unable to open file: " + path.string());
  }

  return {std::istreambuf_iterator<char>(stream),
          std::istreambuf_iterator<char>()};
}

}  // namespace

TEST(CommandTest, EchoPrintsArguments) {
  EchoCommand command({"hello", "world"});
  DummyInput input;
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "hello world\n");
}

TEST(CommandTest, CatPrintsFileContent) {
  const auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  CatCommand command({file.string()});
  DummyInput input;
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), ReadFile(file));
}

TEST(CommandTest, WcReturnsFileStats) {
  const auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  WcCommand command({file.string()});
  DummyInput input;
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "15 98 1039 " + file.string() + "\n");
}

TEST(CommandTest, PwdPrintsCurrentWorkingDirectory) {
  PwdCommand command;
  DummyInput input;
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  auto expected = std::filesystem::current_path().string();
  expected.push_back('\n');
  EXPECT_EQ(output.read(), expected);
}

TEST(CommandTest, ExitCommandSetsExitFlag) {
  IsExit = false;
  ExitCommand command;
  DummyInput input;
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_TRUE(IsExit.load());
  IsExit = false;
}

}  // namespace coreutils::test

