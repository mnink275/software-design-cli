#pragma once

#include <vector>

namespace coreutils {

class Input {
 public:
  virtual ~Input() = default;

  virtual std::vector<char> read(size_t size) = 0;
};

}  // namespace coreutils
