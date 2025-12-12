#pragma once

#include <input.hpp>
#include <output.hpp>

namespace coreutils {

class Command {
 public:
  virtual ~Command() = default;

  virtual int run(Input& in, Output& out) = 0;
};

}  // namespace coreutils
