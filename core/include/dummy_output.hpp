#pragma once

#include <output.hpp>

namespace coreutils {

class DummyOutput final : public Output {
 public:
  void write(std::vector<char>&& data) override {}
};

}  // namespace coreutils
