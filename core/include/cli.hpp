#pragma once

#include <string>
#include <vector>

#include <executor.hpp>
#include <input.hpp>
#include <output.hpp>
#include <parser.hpp>

namespace coreutils {

class Command;

class CLI final {
 public:
  using CommandPtr = std::unique_ptr<Command>;

 public:
  explicit CLI(Parser& parser);

  void runCli(Input& in, Output& out);

 private:
  int process(std::string&& line, Output& out, Input& in);
  std::vector<CommandPtr> splitIntoCommands(std::vector<std::string>&& tokens);

  static CommandPtr createCommand(std::vector<std::string>&& tokens);

  Parser parser_;
  Executor executor_;
};

}  // namespace coreutils
