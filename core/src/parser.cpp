#include <parser.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace coreutils {

namespace {

bool isValidVarNameChar(char ch) { return std::isalnum(ch) || ch == '_'; }

bool isInvalidBeforeEquals(char ch) {
  return std::isspace(ch) || ch == '"' || ch == '\'' || ch == '$' ||
         ch == '&' || ch == '|';
}

void skipWhitespace(std::string_view str, size_t& pos) {
  pos = str.find_first_not_of(" \t\n\r", pos);
  if (pos == std::string_view::npos) {
    pos = str.size();
  }
}

void pushToken(std::string& current_token, std::vector<std::string>& tokens) {
  if (!current_token.empty()) {
    tokens.push_back(std::move(current_token));
    current_token.clear();
  }
}

bool handleOperators(std::string_view input, size_t& pos,
                     std::string& current_token,
                     std::vector<std::string>& tokens) {
  char ch = input[pos];

  if (ch == '&' && pos + 1 < input.size() && input[pos + 1] == '&') {
    pushToken(current_token, tokens);
    tokens.emplace_back("&&");
    ++pos;
    return true;
  }

  if (ch == '|') {
    pushToken(current_token, tokens);
    tokens.emplace_back("|");
    return true;
  }

  return false;
}

std::string extractVariableName(std::string_view input, size_t& pos) {
  if (pos >= input.size()) return "";

  // Формат ${VAR}
  if (input[pos] == '{') {
    ++pos;
    size_t end = input.find('}', pos);
    if (end == std::string_view::npos) {
      end = input.size();
    }
    std::string var_name(input.substr(pos, end - pos));
    pos = end;  // pos будет указывать на '}', цикл сам сделает ++pos
    return var_name;
  }

  // Формат $VAR
  size_t start = pos;
  while (pos < input.size() && isValidVarNameChar(input[pos])) {
    ++pos;
  }
  --pos;
  return std::string(input.substr(start, pos - start + 1));
}

}  // namespace

bool Parser::tryParseAssignment(std::string_view input, size_t& pos) {
  skipWhitespace(input, pos);
  if (pos >= input.size()) return false;

  size_t eq_pos = input.find('=', pos);
  if (eq_pos == std::string_view::npos || eq_pos == pos) {
    return false;
  }

  // Проверяем корректность имени переменной
  std::string_view var_name_part = input.substr(pos, eq_pos - pos);
  if (std::ranges::any_of(var_name_part, isInvalidBeforeEquals)) {
    return false;
  }

  std::string var_name(var_name_part);

  // Извлекаем значение до пробела или конца строки
  size_t value_start = eq_pos + 1;
  size_t value_end = input.find_first_of(" \t\n\r", value_start);
  if (value_end == std::string_view::npos) {
    value_end = input.size();
  }

  env_variables_[var_name] =
      std::string(input.substr(value_start, value_end - value_start));
  pos = value_end;
  return true;
}

// Обработка символа '$' и раскрытие переменной
void Parser::handleDollarSign(std::string_view input, size_t& pos,
                              std::string& current_token) {
  ++pos;
  std::string var_name = extractVariableName(input, pos);
  current_token += expandVariables(var_name, true);
}

std::vector<std::string> Parser::parseToTokens(std::string&& raw_input) {
  std::vector<std::string> tokens;
  size_t pos = 0;

  while (tryParseAssignment(raw_input, pos)) {
    // Ждём пока находим присваивания
  }

  // Основной парсинг
  std::string current_token;
  bool in_single_quote = false;
  bool in_double_quote = false;

  for (; pos < raw_input.size(); ++pos) {
    char ch = raw_input[pos];

    if (in_single_quote) {
      if (ch == '\'') {
        in_single_quote = false;
      } else if (ch == '$') {
        handleDollarSign(raw_input, pos, current_token);
      } else {
        current_token += ch;
      }
      continue;
    }

    if (in_double_quote) {
      if (ch == '"') {
        in_double_quote = false;
      } else if (ch == '$') {
        handleDollarSign(raw_input, pos, current_token);
      } else {
        current_token += ch;
      }
      continue;
    }

    // Вне кавычек
    if (ch == '\'') {
      in_single_quote = true;
    } else if (ch == '"') {
      in_double_quote = true;
    } else if (ch == '$') {
      handleDollarSign(raw_input, pos, current_token);
    } else if (std::isspace(static_cast<unsigned char>(ch))) {
      pushToken(current_token, tokens);
    } else if (handleOperators(raw_input, pos, current_token, tokens)) {
      // Оператор обработан
    } else {
      current_token += ch;
    }
  }

  pushToken(current_token, tokens);
  return tokens;
}

std::string Parser::expandVariables(const std::string& str, bool expand) {
  if (!expand) {
    return str;
  }

  if (auto it = env_variables_.find(str); it != env_variables_.end()) {
    return it->second;
  }

  if (const char* env_value = std::getenv(str.c_str())) {
    return env_value;
  }

  return "";
}

}  // namespace coreutils
