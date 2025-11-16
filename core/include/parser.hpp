#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace coreutils {

class Parser final {
 public:
  std::vector<std::string> parseToTokens(std::string&& raw_input);

 private:
  std::string expandVariables(const std::string& str, bool expand);

 private:
  std::unordered_map<std::string, std::string> env_variables_;
};

}  // namespace coreutils
