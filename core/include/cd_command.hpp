#pragma once

#include <command.hpp>

#include <optional>
#include <string>
#include <vector>

namespace coreutils {

class CdCommand final : public Command {
 public:
  explicit CdCommand(std::vector<std::string> args);

  int run(Input& in, Output& out) override;

 private:
  std::optional<std::string> target_;
};

}  // namespace coreutils

