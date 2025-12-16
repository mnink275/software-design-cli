#include <pipe.hpp>

#include <filesystem>
#include <vector>

#include <cat_command.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <external_command.hpp>
#include <global_state.hpp>
#include <text_input.hpp>
#include <text_output.hpp>
#include <wc_command.hpp>

#include <gtest/gtest.h>

namespace coreutils::test {

TEST(Pipe, Basic) {
  auto [in, out] = createPipe();
  out->write(std::vector{'a'});
  out->write(std::vector{'b', 'c'});
  std::vector<char> expected = {'a', 'b', 'c'};
  EXPECT_EQ(in->readVector(DEFAULT_BLOCK_SIZE), expected);
  out.reset();
  EXPECT_EQ(in->readVector(DEFAULT_BLOCK_SIZE), std::vector<char>());
}

TEST(Pipe, EchoToWc) {
  auto [in, out] = createPipe();

  EchoCommand echo({"Hello", "World"});
  WcCommand wc({});

  TextInput dummy_in("");
  TextOutput final_out;

  ASSERT_EQ(echo.run(dummy_in, *out), 0);
  out.reset();

  ASSERT_EQ(wc.run(*in, final_out), 0);
  EXPECT_EQ(final_out.read(), "       1       2      12\n");
}

TEST(Pipe, CatToWcWithFile) {
  auto [in, out] = createPipe();

  const auto file = std::filesystem::path(TEST_DATA_DIR) / "file.txt";
  CatCommand cat({file.string()});
  WcCommand wc({});

  TextInput dummy_in("");
  TextOutput final_out;

  ASSERT_EQ(cat.run(dummy_in, *out), 0);
  out.reset();

  ASSERT_EQ(wc.run(*in, final_out), 0);
  EXPECT_EQ(final_out.read(), "      15      98    1039\n");
}

TEST(Pipe, ExitInPipeline) {
  auto [in, out] = createPipe();

  IsExit = false;
  ExitCommand exit_cmd;
  EchoCommand echo({"test"});

  TextInput dummy_in("");
  TextOutput final_out;

  ASSERT_EQ(exit_cmd.run(dummy_in, *out), 0);
  EXPECT_TRUE(IsExit.load());
  out.reset();

  ASSERT_EQ(echo.run(*in, final_out), 0);

  IsExit = false;
}

TEST(Pipe, NonZeroExitCode) {
  auto [in, out] = createPipe();

  ExternalCommand failing_cmd("cat", {"non_existent_file.txt"});
  EchoCommand echo({"test"});

  TextInput dummy_in("");
  TextOutput final_out;

  int exit_code = failing_cmd.run(dummy_in, *out);
  EXPECT_NE(exit_code, 0);
  out.reset();

  ASSERT_EQ(echo.run(*in, final_out), 0);
}

TEST(Pipe, ThreeCommandPipeline) {
  // echo "test data" | cat | wc
  auto [in1, out1] = createPipe();
  auto [in2, out2] = createPipe();

  EchoCommand echo({"test", "data"});
  CatCommand cat({});
  WcCommand wc({});

  TextInput dummy_in("");
  TextOutput final_out;

  // echo -> pipe1
  ASSERT_EQ(echo.run(dummy_in, *out1), 0);
  out1.reset();

  // pipe1 -> cat -> pipe2
  ASSERT_EQ(cat.run(*in1, *out2), 0);
  out2.reset();

  // pipe2 -> wc -> output
  ASSERT_EQ(wc.run(*in2, final_out), 0);
  EXPECT_EQ(final_out.read(), "       1       2      10\n");
}

TEST(Pipe, EmptyPipelineInput) {
  auto [in, out] = createPipe();

  out.reset();

  WcCommand wc({});
  TextOutput final_out;

  ASSERT_EQ(wc.run(*in, final_out), 0);
  EXPECT_EQ(final_out.read(), "       0       0       0\n");
}

}  // namespace coreutils::test
