#include <cli.hpp>

#include <sstream>
#include <stdexcept>

#include <command.hpp>
#include <dummy_input.hpp>
#include <external_command.hpp>
#include <global_state.hpp>

namespace coreutils {

CLI::CLI(Parser& parser) : parser_(parser) {}

void CLI::runCli(Input& in, Output& out) {
  constexpr size_t kBatchSize = 1024;

  while (!IsExit) {
    auto data = in.read(kBatchSize);
    std::string str_data{data.begin(), data.end()};
    std::istringstream input_steam{std::move(str_data)};

    std::string line;
    while (std::getline(input_steam, line)) {
      auto res = process(std::move(line), out);

      if (res < 0) {
        throw std::runtime_error{
            "Error has occured during the last process call"};
      }
    }
  }
}

int CLI::process(std::string&& line, Output& out) {
  auto tokens = parser_.parseToTokens(std::move(line));
  auto commands = splitIntoCommands(std::move(tokens));

  DummyInput dummy_input;

  return executor_.runCommands(std::move(commands), dummy_input, out);
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

  return std::make_unique<ExternalCommand>(std::move(cmd_name),
                                           std::move(rest));
}

}  // namespace coreutils
