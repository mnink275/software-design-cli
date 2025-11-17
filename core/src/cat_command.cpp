#include <cat_command.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

namespace coreutils {

namespace {

constexpr size_t kBufferSize = 4096;

}

int CatCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    std::string line;
    while (std::getline(std::cin, line)) {
      line.push_back('\n');
      out.write({line.begin(), line.end()});
    }
    if (std::cin.eof() && isatty(STDIN_FILENO) != 0) {
      std::cin.clear();
    }
    if (!line.empty() && line.back() != '\n') {
      line.push_back('\n');
      out.write({line.begin(), line.end()});
    }
    return 0;
  }

  int exit_code = 0;
  for (const auto& file : files_) {
    std::ifstream stream(file, std::ios::binary);
    if (!stream.is_open()) {
      std::cerr << "Unable to open file: " << file << '\n';
      exit_code = 1;
      continue;
    }

    std::vector<char> buffer(kBufferSize);
    while (stream) {
      stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
      const auto read_count = stream.gcount();
      if (read_count <= 0) {
        break;
      }
      out.write(
          {buffer.data(), buffer.data() + static_cast<size_t>(read_count)});
    }
  }

  return exit_code;
}

}  // namespace coreutils

