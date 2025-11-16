#include <external_command.hpp>

#include <dummy_input.hpp>
#include <dummy_output.hpp>
#include <pipe.hpp>
#include <text_output.hpp>

#include <gtest/gtest.h>

namespace coreutils::test {

TEST(ExternalCommand, Basic) {
  ExternalCommand cmd("echo", {"Test", "External Command"});
  DummyInput in;
  TextOutput out;
  ASSERT_EQ(cmd.run(in, out), 0);
  ASSERT_EQ(out.read(), "Test External Command\n");
}

TEST(ExternalCommand, ProcessInteraction) {
  ExternalCommand cat("cat", {"../../test/data/file.txt"});
  ExternalCommand wc("wc", {});
  DummyInput in;
  auto [pipeIn, pipeOut] = createPipe();
  TextOutput out;
  ASSERT_EQ(cat.run(in, *pipeOut), 0);
  pipeOut.reset();
  ASSERT_EQ(wc.run(*pipeIn, out), 0);
  ASSERT_EQ(out.read(), "     15      98    1039\n");
  // ASSERT_EQ(out.read(), "      15     100    1039\n"); // На MacOS почему-то так
}

TEST(ExternalCommand, Error) {
  ExternalCommand cmd("cat", {"not_existing_file_path"});
  DummyInput in;
  DummyOutput out;
  ASSERT_EQ(cmd.run(in, out), 1);
}

}  // namespace coreutils::test
