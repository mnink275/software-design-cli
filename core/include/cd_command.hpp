#pragma once
#include <command.hpp>
#include <string>
#include <vector>

namespace coreutils {

class CdCommand : public Command {
 public:
  explicit CdCommand(std::vector<std::string> args);
  int run(Input& in, Output& out) override;

 private:
  std::vector<std::string> args_;
};

}  // namespace coreutils
