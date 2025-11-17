#pragma once

#include <command.hpp>
#include <global_state.hpp>

namespace coreutils {

class ExitCommand final : public Command {
 public:
  int run(Input& , Output& ) override {
    IsExit = true;
    return 0;
  }
};

}  // namespace coreutils

