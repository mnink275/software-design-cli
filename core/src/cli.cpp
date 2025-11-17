#include <cli.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include <cat_command.hpp>
#include <command.hpp>
#include <dummy_input.hpp>
#include <echo_command.hpp>
#include <exit_command.hpp>
#include <external_command.hpp>
#include <global_state.hpp>
#include <pwd_command.hpp>
#include <wc_command.hpp>

namespace coreutils {

CLI::CLI(Parser& parser) : parser_(parser) {}

void CLI::runCli(Input& in, Output& out) {
  constexpr size_t kBatchSize = 1024;
  bool is_tty = isatty(STDIN_FILENO) != 0;

  while (!IsExit) {
    std::cout << "-> ";
    std::cout.flush();

    auto data = in.read(kBatchSize);
    if (data.empty()) {
      if (std::cin.eof()) {
        break;
      }
      continue;
    }
    std::string str_data{data.begin(), data.end()};
    std::istringstream input_steam{std::move(str_data)};

    std::string line;
    while (std::getline(input_steam, line)) {
      auto res = process(std::move(line), out, in);

      if (res < 0) {
        throw std::runtime_error{
            "Error has occured during the last process call"};
      }
    }
  }
}

int CLI::process(std::string&& line, Output& out, Input& in) {
  auto tokens = parser_.parseToTokens(std::move(line));

  if (tokens.empty()) return 0;

  auto commands = splitIntoCommands(std::move(tokens));

  bool needs_stdin = false;
  if (!commands.empty()) {
    needs_stdin = false;
  }
  DummyInput dummy_input;
  Input& cmd_input = dummy_input;

  try {
    return executor_.runCommands(std::move(commands), cmd_input, out);
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}

std::vector<CLI::CommandPtr> CLI::splitIntoCommands(
    std::vector<std::string>&& tokens) {
  auto cmd = createCommand(std::move(tokens));

  std::vector<CLI::CommandPtr> result;
  result.push_back(std::move(cmd));
  return result;
}

CLI::CommandPtr CLI::createCommand(std::vector<std::string>&& tokens) {
  std::string cmd_name = std::move(tokens[0]);

  std::vector<std::string> rest(std::make_move_iterator(tokens.begin() + 1),
                                std::make_move_iterator(tokens.end()));

  if (cmd_name == "echo") {
    return std::make_unique<EchoCommand>(std::move(rest));
  }

  if (cmd_name == "cat") {
    // Cat reads from stdin
    return std::make_unique<CatCommand>(std::move(rest));
  }

  if (cmd_name == "wc") {
    // Wc reads from stdin
    return std::make_unique<WcCommand>(std::move(rest));
  }

  if (cmd_name == "pwd") {
    if (!rest.empty()) {
      throw std::invalid_argument("pwd does not accept arguments");
    }
    return std::make_unique<PwdCommand>();
  }

  if (cmd_name == "exit") {
    // Exit command ignores arguments
    return std::make_unique<ExitCommand>();
  }

  return std::make_unique<ExternalCommand>(std::move(cmd_name),
                                           std::move(rest));
}

}  // namespace coreutils
