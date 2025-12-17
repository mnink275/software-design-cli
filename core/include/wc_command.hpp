#pragma once

#include <command.hpp>

#include <string>
#include <vector>

namespace coreutils {

class WcCommand final : public Command {
 public:
  explicit WcCommand(std::vector<std::string> args);

  int run(Input& in, Output& out) override;

 private:
  std::vector<std::string> files_;
  bool count_lines_{false};
  bool count_words_{false};
  bool count_bytes_{false};
};

}  // namespace coreutils
