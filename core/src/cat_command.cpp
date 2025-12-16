#include <cat_command.hpp>

#include <fstream>
#include <iostream>
#include <vector>

namespace coreutils {

namespace {

constexpr size_t kBufferSize = 4096;

}

int CatCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    auto buf = in.readVector(kBufferSize);
    while (!buf.empty()) {
      out.write(buf);
      buf = in.readVector(kBufferSize);
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
      out.write(buffer.data(), read_count);
    }
  }

  return exit_code;
}

}  // namespace coreutils
