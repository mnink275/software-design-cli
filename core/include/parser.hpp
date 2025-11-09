#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace coreutils {

class Parser final {
 public:
  std::vector<std::string> parceToTokens(std::string&& raw_input);

 private:
  std::unordered_map<std::string, std::string> env_variables_;
};

}  // namespace coreutils
