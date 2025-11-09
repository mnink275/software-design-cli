#include <cli.hpp>
#include <dummy_output.hpp>
#include <dummy_input.hpp>

#include <string>
#include <iostream>

int main() {
  coreutils::Parser parser;
  coreutils::DummyOutput output;
  coreutils::DummyInput input;

  coreutils::CLI cli{parser};

  try {
    cli.runCli(input, output);
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return -1;
  }
}
