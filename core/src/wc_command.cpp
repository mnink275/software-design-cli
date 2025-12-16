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

std::string toString(const FileStats& stats, bool lines, bool words,
                     bool bytes) {
  std::string result;
  constexpr int kFieldWidth = 8;

  if (lines) {
    std::string line_str = std::to_string(stats.lines);
    result += std::string(kFieldWidth - line_str.length(), ' ') + line_str;
  }
  if (words) {
    std::string word_str = std::to_string(stats.words);
    result += std::string(kFieldWidth - word_str.length(), ' ') + word_str;
  }
  if (bytes) {
    std::string byte_str = std::to_string(stats.bytes);
    result += std::string(kFieldWidth - byte_str.length(), ' ') + byte_str;
  }
  return result;
}

}  // namespace

WcCommand::WcCommand(std::vector<std::string> args) {
  for (const auto& arg : args) {
    if (arg == "-l") {
      count_lines_ = true;
    } else if (arg == "-w") {
      count_words_ = true;
    } else if (arg == "-c") {
      count_bytes_ = true;
    } else if (!arg.empty() && arg[0] != '-') {
      files_.push_back(arg);
    }
  }

  if (!count_lines_ && !count_words_ && !count_bytes_) {
    count_lines_ = true;
    count_words_ = true;
    count_bytes_ = true;
  }
}

int WcCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    FileStats stats =
        collectStats([&in](size_t size) { return in.readVector(size); });

    auto result =
        toString(stats, count_lines_, count_words_, count_bytes_) + "\n";
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
      auto line = toString(stats, count_lines_, count_words_, count_bytes_) +
                  " " + file + "\n";
      out.write(line);
    } catch (const std::runtime_error&) {
      std::cerr << "Unable to open file: " << file << '\n';
      exit_code = 1;
    }
  }

  return exit_code;
}

}  // namespace coreutils
