#include <parser.hpp>

#include <cctype>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

namespace coreutils {

std::string Parser::expandVariables(const std::string& str, bool expand) {
  if (!expand) {
    return str;
  }

  std::string result;
  size_t i = 0;

  while (i < str.size()) {
    if (str[i] == '$' && i + 1 < str.size()) {
      i++;  // пропускаем $
      std::string var_name;

      // ${VAR} or $VAR
      if (str[i] == '{') {
        i++;
        while (i < str.size() && str[i] != '}') {
          var_name += str[i++];
        }
        if (i < str.size()) i++;  // пропускаем }
      } else {
        while (i < str.size() &&
               ((std::isalnum(str[i]) != 0) || str[i] == '_')) {
          var_name += str[i++];
        }
      }

      if (env_variables_.contains(var_name)) {
        result += env_variables_[var_name];
      } else if (const char* val = std::getenv(var_name.c_str())) {
        result += val;
      }
    } else {
      result += str[i++];
    }
  }

  return result;
}

std::vector<std::string> Parser::parseToTokens(std::string&& raw_input) {
  std::vector<std::string> tokens;
  std::string current_token;

  bool in_single_quote = false;
  bool in_double_quote = false;

  for (char c : raw_input) {
    if (in_single_quote) {
      if (c == '\'') {
        in_single_quote = false;
      } else {
        current_token += c;
      }
    } else if (in_double_quote) {
      if (c == '"') {
        in_double_quote = false;
      } else {
        current_token += c;
      }
    } else {
      if (c == '\'') {
        in_single_quote = true;
      } else if (c == '"') {
        in_double_quote = true;
      } else if (std::isspace(c) != 0) {
        if (!current_token.empty()) {
          tokens.push_back(expandVariables(current_token, true));
          current_token.clear();
        }
      } else {
        current_token += c;
      }
    }
  }

  if (!current_token.empty()) {
    tokens.push_back(expandVariables(current_token, true));
  }

  return tokens;
}

}  // namespace coreutils
