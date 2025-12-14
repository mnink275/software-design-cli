#pragma once

#include <command.hpp>

#include <optional>
#include <string>
#include <vector>

namespace coreutils {

class LsCommand final : public Command {
 public:
  explicit LsCommand(std::vector<std::string> args);

  int run(Input& in, Output& out) override;

 private:
  std::optional<std::string> target_;
};

}  // namespace coreutils

