#include <pipe.hpp>

#include <vector>

#include <gtest/gtest.h>

namespace coreutils::test {

TEST(Pipe, Basic) {
  auto [in, out] = createPipe();
  out->write({'a'});
  out->write({'b', 'c'});
  std::vector<char> expected = {'a', 'b', 'c'};
  EXPECT_EQ(in->read(DEFAULT_BLOCK_SIZE), expected);
  out.reset();
  EXPECT_EQ(in->read(DEFAULT_BLOCK_SIZE), std::vector<char>());
}

}  // namespace coreutils::test
