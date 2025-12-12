#pragma once

#include <memory>
#include <vector>

namespace coreutils {

class Command;
class Input;
class Output;

class Executor final {
 public:
  using CommandPtr = std::unique_ptr<Command>;

 public:
  int runCommands(std::vector<CommandPtr> cmds, Input& in, Output& out);
};

}  // namespace coreutils
