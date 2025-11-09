#pragma once

#include <input.hpp>

namespace coreutils {

class DummyInput final : public Input {
 public:
  std::vector<char> read(size_t size) override { return {}; }
};

}  // namespace coreutils
