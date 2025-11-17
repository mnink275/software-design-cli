#include <wc_command.hpp>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

namespace coreutils {

namespace {

constexpr size_t kBufferSize = 4096;

}  // namespace

int WcCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    FileStats stats{};
    bool in_word = false;
    std::string line;
    bool has_content = false;
    
    while (std::getline(std::cin, line)) {
      has_content = true;
      ++stats.lines;
      stats.bytes += line.size() + 1;
      for (char ch : line) {
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
          if (in_word) {
            ++stats.words;
            in_word = false;
          }
        } else {
          in_word = true;
        }
      }
      if (in_word) {
        ++stats.words;
        in_word = false;
      }
    }
    if (std::cin.eof() && isatty(STDIN_FILENO) != 0) {
      std::cin.clear();
    }
    if (!line.empty() && !has_content) {
      stats.bytes += line.size();
      for (char ch : line) {
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
          if (in_word) {
            ++stats.words;
            in_word = false;
          }
        } else {
          in_word = true;
        }
      }
      if (in_word) {
        ++stats.words;
      }
    }
    
    auto result = std::to_string(stats.lines) + " " +
                  std::to_string(stats.words) + " " +
                  std::to_string(stats.bytes) + "\n";
    out.write({result.begin(), result.end()});
    return 0;
  }

  int exit_code = 0;
  for (const auto& file : files_) {
    try {
      auto stats = collectStats(file);
      auto line = std::to_string(stats.lines) + " " +
                  std::to_string(stats.words) + " " +
                  std::to_string(stats.bytes) + " " + file + "\n";
      out.write({line.begin(), line.end()});
    } catch (const std::runtime_error&) {
      std::cerr << "Unable to open file: " << file << '\n';
      exit_code = 1;
    }
  }

  return exit_code;
}

WcCommand::FileStats WcCommand::collectStats(const std::string& file) const {
  std::ifstream stream(file, std::ios::binary);
  if (!stream.is_open()) {
    throw std::runtime_error("Unable to open file: " + file);
  }

  FileStats stats{};
  bool in_word = false;
  std::vector<char> buffer(kBufferSize);
  while (stream) {
    stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    const auto read_count = stream.gcount();
    if (read_count <= 0) {
      break;
    }

    stats.bytes += static_cast<size_t>(read_count);
    for (std::streamsize i = 0; i < read_count; ++i) {
      const unsigned char ch =
          static_cast<unsigned char>(buffer[static_cast<size_t>(i)]);
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
  }

  if (in_word) {
    ++stats.words;
  }

  return stats;
}

}  // namespace coreutils

