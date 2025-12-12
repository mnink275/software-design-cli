#pragma once

#include <command.hpp>

#include <string>
#include <vector>

namespace coreutils {

class EchoCommand final : public Command {
 public:
  explicit EchoCommand(std::vector<std::string> args)
      : args_(std::move(args)) {}

  int run(Input& in, Output& out) override;

 private:
  std::vector<std::string> args_;
};

}  // namespace coreutils

