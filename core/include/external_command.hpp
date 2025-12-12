#pragma once

#include <command.hpp>

#include <unistd.h>

#include <string>
#include <vector>

namespace coreutils {

class ExternalCommand : public Command {
 public:
  ExternalCommand(std::string command, std::vector<std::string> args)
      : command_(std::move(command)), args_(std::move(args)) {}
  int run(Input& in, Output& out) override;

 private:
  std::string command_;
  std::vector<std::string> args_;
  pid_t child_{};
};

}  // namespace coreutils