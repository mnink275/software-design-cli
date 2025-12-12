#include <cli.hpp>
#include <std_output.hpp>
#include <std_input.hpp>

#include <iostream>

int main() {
  coreutils::Parser parser;
  coreutils::StdOutput output;
  coreutils::StdInput input;

  coreutils::CLI cli{parser};

  try {
    cli.runCli(input, output);
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return -1;
  }
}
