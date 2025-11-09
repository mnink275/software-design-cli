#pragma once

#include <vector>

namespace coreutils {

class Output {
 public:
  virtual ~Output() = default;

  virtual void write(std::vector<char>&& data) = 0;
};

}  // namespace coreutils
