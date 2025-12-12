#include <wc_command.hpp>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace coreutils {

namespace {

constexpr size_t kBufferSize = 4096;

struct FileStats {
  size_t lines{};
  size_t words{};
  size_t bytes{};
};

[[nodiscard]] FileStats collectStats(auto reader) {
  FileStats stats{};
  bool in_word = false;
  std::vector<char> buffer = reader(kBufferSize);
  while (!buffer.empty()) {
    stats.bytes += static_cast<size_t>(buffer.size());
    for (char ch : buffer) {
      if (ch == '\n') {
        ++stats.lines;
      }

      if (std::isspace(ch) != 0) {
        if (in_word) {
          ++stats.words;
          in_word = false;
        }
      } else {
        in_word = true;
      }
    }

    buffer = reader(kBufferSize);
  }

  if (in_word) {
    ++stats.words;
  }

  return stats;
}

std::string toString(const FileStats& stats) {
  return std::to_string(stats.lines) + " " + std::to_string(stats.words) + " " +
         std::to_string(stats.bytes);
}

}  // namespace

int WcCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    FileStats stats =
        collectStats([&in](size_t size) { return in.readVector(size); });

    auto result = toString(stats) + "\n";
    out.write(result);
    return 0;
  }

  int exit_code = 0;
  for (const auto& file : files_) {
    try {
      std::ifstream stream(file, std::ios::binary);
      if (!stream.is_open()) {
        throw std::runtime_error("Unable to open file: " + file);
      }
      auto stats =
          collectStats([stream = std::move(stream)](size_t size) mutable {
            std::vector<char> buf(size);
            stream.read(buf.data(), static_cast<std::streamsize>(size));
            buf.resize(stream.gcount());
            return buf;
          });
      auto line = toString(stats) + " " + file + "\n";
      out.write(line);
    } catch (const std::runtime_error&) {
      std::cerr << "Unable to open file: " << file << '\n';
      exit_code = 1;
    }
  }

  return exit_code;
}

}  // namespace coreutils
