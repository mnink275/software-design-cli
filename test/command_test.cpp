#include <gtest/gtest.h>

#include <cat_command.hpp>
#include <cd_command.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <global_state.hpp>
#include <ls_command.hpp>
#include <pwd_command.hpp>
#include <text_input.hpp>
#include <text_output.hpp>
#include <wc_command.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <optional>
#include <cstdlib>
#include <vector>

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

std::filesystem::path MakeTempPath(const std::string& prefix) {
  static size_t counter = 0;
  auto base = std::filesystem::temp_directory_path();
  auto name = prefix + "-" + std::to_string(++counter);
  return base / name;
}

std::filesystem::path CreateTempDirectory(const std::string& prefix) {
  auto dir = MakeTempPath(prefix);
  std::filesystem::remove_all(dir);
  std::filesystem::create_directories(dir);
  return dir;
}

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

class ScopedEnvVar {
 public:
  ScopedEnvVar(std::string name, std::string value)
      : name_(std::move(name)), value_(std::move(value)) {
    if (const char* current = std::getenv(name_.c_str())) {
      old_value_ = current;
    }
    setenv(name_.c_str(), value_.c_str(), 1);
  }
  ScopedEnvVar(const ScopedEnvVar&) = delete;
  ScopedEnvVar& operator=(const ScopedEnvVar&) = delete;
  ScopedEnvVar(ScopedEnvVar&&) = delete;
  ScopedEnvVar& operator=(ScopedEnvVar&&) = delete;
  ~ScopedEnvVar() {
    if (old_value_) {
      setenv(name_.c_str(), old_value_->c_str(), 1);
    } else {
      unsetenv(name_.c_str());
    }
  }

 private:
  std::string name_;
  std::string value_;
  std::optional<std::string> old_value_;
};

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
  EXPECT_EQ(output.read(), "15 98 1039 " + file.string() + "\n");
}

TEST(CommandTest, WcReadInput) {
  WcCommand command({});

  std::string file(ReadFile(std::filesystem::path(TEST_DATA_DIR) / "file.txt"));

  TextInput input(std::move(file));
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "15 98 1039\n");
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

TEST(CommandTest, CdChangesWorkingDirectory) {
  ScopedChdir guard;
  const auto target = CreateTempDirectory("cd-command");

  CdCommand command(std::vector<std::string>{target.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(std::filesystem::current_path(), target);

  std::filesystem::remove_all(target);
}

TEST(CommandTest, CdDefaultsToHomeWithoutArguments) {
  const auto home_dir = CreateTempDirectory("cd-home");

  ScopedChdir guard;
  ScopedEnvVar home("HOME", home_dir.string());

  CdCommand command({});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(std::filesystem::current_path(), home_dir);

  std::filesystem::remove_all(home_dir);
}

TEST(CommandTest, CdFailsOnMissingDirectory) {
  ScopedChdir guard;
  const auto missing = MakeTempPath("missing-directory");

  CdCommand command(std::vector<std::string>{missing.string()});
  TextInput input("");
  TextOutput output;

  EXPECT_EQ(command.run(input, output), 1);
  EXPECT_NE(std::filesystem::current_path(), missing);
}

TEST(CommandTest, LsPrintsDirectoryEntries) {
  const auto dir = CreateTempDirectory("ls-test");

  std::ofstream(dir / "alpha.txt").put('a');
  std::ofstream(dir / "beta.txt").put('b');
  std::filesystem::create_directories(dir / "subdir");

  LsCommand command(std::vector<std::string>{dir.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "alpha.txt\nbeta.txt\nsubdir\n");

  std::filesystem::remove_all(dir);
}

TEST(CommandTest, LsPrintsFileNameWhenArgumentIsFile) {
  const auto file =
      std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  LsCommand command(std::vector<std::string>{file.string()});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), file.string() + "\n");
}

}  // namespace coreutils::test
