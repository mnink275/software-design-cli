#pragma once

#include <input.hpp>

#include <unistd.h>

namespace coreutils {

class StdInput final : public Input {
 public:
  [[nodiscard]] int fd() const override { return STDIN_FILENO; }
};

}  // namespace coreutils
