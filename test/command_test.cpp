#include <gtest/gtest.h>

#include <cat_command.hpp>
#include <cd_command.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <global_state.hpp>
#include <ls_command.hpp>
#include <grep_command.hpp>
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

TEST(GrepTest, BasicPatternMatch) {
  GrepCommand command({"hello", std::filesystem::path(TEST_DATA_DIR) / "file.txt"});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "");
}

TEST(GrepTest, MatchesLinesWithPattern) {
  GrepCommand command({"Штирлиц", std::filesystem::path(TEST_DATA_DIR) / "file.txt"});
  TextInput input("");
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("Штирлиц") != std::string::npos);
}

TEST(GrepTest, CaseInsensitiveSearch) {
  std::string test_input = "Hello World\nhello world\nHELLO WORLD\nfoo bar\n";
  GrepCommand command({"-i", "hello"});
  TextInput input(test_input);
  TextOutput output;
  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("Hello World") != std::string::npos);
  EXPECT_TRUE(result.find("hello world") != std::string::npos);
  EXPECT_TRUE(result.find("HELLO WORLD") != std::string::npos);
  EXPECT_TRUE(result.find("foo bar") == std::string::npos);
}

TEST(GrepTest, WholeWordSearch) {
  std::string test_input = "test testing tester\ntest only\ncontest\n";
  GrepCommand command({"-w", "test"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("test testing tester") != std::string::npos);
  EXPECT_TRUE(result.find("test only") != std::string::npos);
}

TEST(GrepTest, WholeWordSearchStrict) {
  std::string test_input = "contest\nprotester\n";
  GrepCommand command({"-w", "test"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "");
}

TEST(GrepTest, AfterContextZero) {
  std::string test_input = "line1\nmatch\nline3\nline4\n";
  GrepCommand command({"-A", "0", "match"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "match\n");
}

TEST(GrepTest, AfterContextOne) {
  std::string test_input = "line1\nmatch\nline3\nline4\n";
  GrepCommand command({"-A", "1", "match"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "match\nline3\n");
}

TEST(GrepTest, AfterContextTwo) {
  std::string test_input = "line1\nmatch\nline3\nline4\nline5\n";
  GrepCommand command({"-A", "2", "match"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "match\nline3\nline4\n");
}

TEST(GrepTest, OverlappingContextAreas) {
  std::string test_input = "line1\nmatch1\nline3\nmatch2\nline5\nline6\n";
  GrepCommand command({"-A", "2", "match"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("match1") != std::string::npos);
  EXPECT_TRUE(result.find("match2") != std::string::npos);
  EXPECT_TRUE(result.find("line3") != std::string::npos);
  EXPECT_TRUE(result.find("line5") != std::string::npos);
  EXPECT_TRUE(result.find("line6") != std::string::npos);
}

TEST(GrepTest, RegexAnchorEnd) {
  std::string test_input = "hello world\nworld hello\nhello\n";
  GrepCommand command({"world$"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "hello world\n");
}

TEST(GrepTest, RegexAnchorStart) {
  std::string test_input = "hello world\nworld hello\nhello\n";
  GrepCommand command({"^hello"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("hello world") != std::string::npos);
  EXPECT_TRUE(result.find("hello\n") != std::string::npos);
  EXPECT_TRUE(result.find("world hello") == std::string::npos);
}

TEST(GrepTest, RegexDotStar) {
  std::string test_input = "abc123def\nfoo\n123\n";
  GrepCommand command({"abc.*def"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_EQ(result, "abc123def\n");
}

TEST(GrepTest, ReadFromStdin) {
  std::string test_input = "first line\nsecond line\nthird line\n";
  GrepCommand command({"second"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "second line\n");
}

TEST(GrepTest, FileNotFound) {
  GrepCommand command({"pattern", "/nonexistent/file.txt"});
  TextInput input("");
  TextOutput output;
  int result = command.run(input, output);
  EXPECT_NE(result, 0);
}

TEST(GrepTest, CombinedFlags) {
  std::string test_input = "TEST word\ntest WORD\nTesting\n";
  GrepCommand command({"-i", "-w", "test"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("TEST word") != std::string::npos);
  EXPECT_TRUE(result.find("test WORD") != std::string::npos);
  EXPECT_TRUE(result.find("Testing") == std::string::npos);
}

TEST(GrepTest, NoMatchReturnsEmpty) {
  std::string test_input = "foo bar baz\n";
  GrepCommand command({"xyz"});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  EXPECT_EQ(output.read(), "");
}

TEST(GrepTest, EmptyPatternMatchesAll) {
  std::string test_input = "line1\nline2\n";
  GrepCommand command({""});
  TextInput input(test_input);
  TextOutput output;

  ASSERT_EQ(command.run(input, output), 0);
  std::string result = output.read();
  EXPECT_TRUE(result.find("line1") != std::string::npos);
  EXPECT_TRUE(result.find("line2") != std::string::npos);
}


}  // namespace coreutils::test
