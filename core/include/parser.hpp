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
  bool tryParseAssignment(std::string_view input, size_t& pos);
  void handleDollarSign(std::string_view input, size_t& pos,
                        std::string& current_token);

 private:
  std::unordered_map<std::string, std::string> env_variables_;
};

}  // namespace coreutils
