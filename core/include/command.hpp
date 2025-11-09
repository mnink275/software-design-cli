#pragma once

namespace coreutils {

class Input;
class Output;

class Command {
 public:
  virtual ~Command() = default;

  virtual int run(Input& in, Output& out) = 0;
};

}  // namespace coreutils
