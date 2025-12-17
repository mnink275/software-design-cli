#include <gtest/gtest.h>

#include <cat_command.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <global_state.hpp>
#include <pwd_command.hpp>
#include <text_input.hpp>
#include <text_output.hpp>
#include <wc_command.hpp>
#include <ls_command.hpp>
#include <cd_command.hpp>



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
  TextInput input("");
  TextOutput output;
  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "hello world\n");
}

TEST(CommandTest, CatPrintsFileContent) {
  const auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  CatCommand command({file.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), ReadFile(file));
}

TEST(CommandTest, CatReadInput) {
  CatCommand command({});

  std::string file(ReadFile(std::filesystem::path(TEST_DATA_DIR) / "file.txt"));

  TextInput input(file);
  TextOutput output;
  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), file);
}

TEST(CommandTest, WcReturnsFileStats) {
  const auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  WcCommand command({file.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "      15      98    1039 " + file.string() + "\n");
}

TEST(CommandTest, WcReadInput) {
  WcCommand command({});

  std::string file(ReadFile(std::filesystem::path(TEST_DATA_DIR) / "file.txt"));

  TextInput input(std::move(file));
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "      15      98    1039\n");
}

TEST(CommandTest, PwdPrintsCurrentWorkingDirectory) {
  PwdCommand command;
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  auto expected = std::filesystem::current_path().string();
  expected.push_back('\n');
  EXPECT_EQ(output.read(), expected);
}

TEST(CommandTest, ExitCommandSetsExitFlag) {
  IsExit = false;
  ExitCommand command;
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_TRUE(IsExit.load());
  IsExit = false;
}
TEST(CommandTest, LsNoArgumentsListsCurrentDirectory) {
  LsCommand command({});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);

  std::vector<std::string> expected;
  for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
    expected.push_back(entry.path().filename().string());
  }
  std::sort(expected.begin(), expected.end());

  std::string expected_str;
  for (const auto& name : expected) {
    expected_str += name + "\n";
  }

  EXPECT_EQ(output.read(), expected_str);
}

TEST(CommandTest, LsWithDirectoryArgumentListsContents) {
  auto dir = std::filesystem::path(TEST_DATA_DIR);
  LsCommand command({dir.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);

  std::vector<std::string> expected;
  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    expected.push_back(entry.path().filename().string());
  }
  std::sort(expected.begin(), expected.end());

  std::string expected_str;
  for (const auto& name : expected) {
    expected_str += name + "\n";
  }

  EXPECT_EQ(output.read(), expected_str);
}

TEST(CommandTest, LsWithFilePrintsFileName) {
  auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  LsCommand command({file.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), file.filename().string() + "\n");
}
TEST(CommandTest, CdChangesCurrentDirectory) {
  auto original = std::filesystem::current_path();
  auto dir = std::filesystem::path(TEST_DATA_DIR);

  CdCommand command({dir.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(std::filesystem::current_path(), dir);

  std::filesystem::current_path(original);
}

TEST(CommandTest, CdWithNoArgumentsGoesToHome) {
  auto original = std::filesystem::current_path();
  std::filesystem::path home = std::getenv("HOME");

  CdCommand command({});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(std::filesystem::current_path(), home);

  std::filesystem::current_path(original);
}

TEST(CommandTest, CdTooManyArgumentsFails) {
  CdCommand command({"arg1", "arg2"});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 1);
}


}  // namespace coreutils::test
