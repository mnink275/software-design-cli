#pragma once

#include <command.hpp>

namespace coreutils {

class PwdCommand final : public Command {
 public:
  int run(Input& in, Output& out) override;
};

}  // namespace coreutils

