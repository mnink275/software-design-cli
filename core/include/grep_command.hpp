#pragma once

#include <command.hpp>

#include <regex>
#include <string>
#include <vector>

namespace coreutils {

class GrepCommand final : public Command {
 public:
  explicit GrepCommand(std::vector<std::string> args);

  int run(Input& in, Output& out) override;

 private:
  void parseArgs(std::vector<std::string> args);
  [[nodiscard]] std::regex buildRegex() const;
  [[nodiscard]] bool matchesLine(const std::string& line,
                                 const std::regex& regex) const;
  int processFile(const std::string& filename, Output& out);
  int processInput(Input& in, Output& out);
  void outputMatchingLines(const std::vector<std::string>& lines,
                           const std::regex& regex, Output& out,
                           const std::string& filename = "") const;

  std::string pattern_;
  std::vector<std::string> files_;
  bool case_insensitive_{false};  // -i flag
  bool whole_word_{false};        // -w flag
  int after_context_{0};          // -A flag
};

}  // namespace coreutils

