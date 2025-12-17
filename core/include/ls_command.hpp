#pragma once
#include <command.hpp>
#include <string>
#include <vector>

namespace coreutils {

class LsCommand : public Command {
 public:
  explicit LsCommand(std::vector<std::string> args);
  int run(Input& in, Output& out) override;

 private:
  std::vector<std::string> paths_;
};

}  // namespace coreutils
