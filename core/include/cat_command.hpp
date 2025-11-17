#pragma once

#include <command.hpp>

#include <string>
#include <vector>

namespace coreutils {

class CatCommand final : public Command {
 public:
  explicit CatCommand(std::vector<std::string> files)
      : files_(std::move(files)) {}

  int run(Input& in, Output& out) override;

 private:
  std::vector<std::string> files_;
};

}  // namespace coreutils

