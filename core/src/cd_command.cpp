#include "cd_command.hpp"
#include <cstdlib>        
#include <filesystem>
#include <iostream>

namespace coreutils {

CdCommand::CdCommand(std::vector<std::string> args) : args_(std::move(args)) {}

int CdCommand::run(Input& /*in*/, Output& /*out*/) {
  if (args_.size() > 1) {
    std::cerr << "cd: too many arguments\n";
    return 1;
  }

  std::string target;
  if (args_.empty()) {
    auto home = std::getenv("HOME");
    if (!home) {
      std::cerr << " no HOME env\n";
      return 1;
    }
    target = std::string(home);
  } else {
    target = args_[0];
  }

  std::error_code ec;
  std::filesystem::current_path(target, ec);
  if (ec) {
    std::cerr << "cd: " << ec.message() << ": " << target << '\n';
    return 1;
  }

  return 0;
}

}  // namespace coreutils
