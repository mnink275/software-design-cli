#pragma once

#include <output.hpp>

#include <unistd.h>

namespace coreutils {

class StdOutput : public Output {
 public:
  [[nodiscard]] int fd() const override { return STDOUT_FILENO; }
};

}  // namespace coreutils
