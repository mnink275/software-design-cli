#include <grep_command.hpp>

#include <CLI11.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace coreutils {

namespace {

constexpr size_t kBufferSize = 4096;

std::vector<std::string> splitIntoLines(const std::string& text) {
  std::vector<std::string> lines;
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    lines.push_back(line);
  }
  return lines;
}

std::string readAllFromInput(Input& in) {
  std::string result;
  auto buffer = in.readVector(kBufferSize);
  while (!buffer.empty()) {
    result.append(buffer.begin(), buffer.end());
    buffer = in.readVector(kBufferSize);
  }
  return result;
}

std::string readAllFromFile(const std::string& filename) {
  std::ifstream stream(filename, std::ios::binary);
  if (!stream.is_open()) {
    throw std::runtime_error("Unable to open file: " + filename);
  }
  std::ostringstream content;
  content << stream.rdbuf();
  return content.str();
}

}  // namespace

GrepCommand::GrepCommand(std::vector<std::string> args) { parseArgs(std::move(args)); }

void GrepCommand::parseArgs(std::vector<std::string> args) {
  CLI::App app{"grep - search for patterns in files"};

  app.add_flag("-i,--ignore-case", case_insensitive_,
               "Ignore case distinctions in patterns and data");
  app.add_flag("-w,--word-regexp", whole_word_,
               "Select only those lines containing matches that form whole words");
  app.add_option("-A,--after-context", after_context_,
                 "Print NUM lines of trailing context after matching lines")
      ->default_val(0)
      ->check(CLI::NonNegativeNumber);

  app.add_option("pattern", pattern_, "The pattern to search for")->required();

  app.add_option("files", files_, "Files to search in");

  std::vector<std::string> argv_strings;
  argv_strings.push_back("grep");
  for (auto& arg : args) {
    argv_strings.push_back(std::move(arg));
  }

  std::vector<char*> argv;
  argv.reserve(argv_strings.size());
  for (auto& s : argv_strings) {
    argv.push_back(s.data());
  }

  try {
    app.parse(static_cast<int>(argv.size()), argv.data());
  } catch (const CLI::ParseError& e) {
    throw std::invalid_argument(app.help());
  }
}

std::regex GrepCommand::buildRegex() const {
  std::string regex_pattern = pattern_;
  if (whole_word_) {
    regex_pattern = "\\b(" + regex_pattern + ")\\b";
  }

  std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
  if (case_insensitive_) {
    flags |= std::regex_constants::icase;
  }

  return std::regex(regex_pattern, flags);
}

bool GrepCommand::matchesLine(const std::string& line,
                              const std::regex& regex) const {
  return std::regex_search(line, regex);
}

void GrepCommand::outputMatchingLines(const std::vector<std::string>& lines,
                                       const std::regex& regex, Output& out,
                                       const std::string& filename) const {
  std::set<size_t> lines_to_print;
  for (size_t i = 0; i < lines.size(); ++i) {
    if (matchesLine(lines[i], regex)) {
      lines_to_print.insert(i);
      for (int j = 1; j <= after_context_ && (i + j) < lines.size(); ++j) {
        lines_to_print.insert(i + j);
      }
    }
  }

  size_t last_printed = static_cast<size_t>(-1);
  bool need_separator = false;

  for (size_t idx : lines_to_print) {
    if (last_printed != static_cast<size_t>(-1) && idx > last_printed + 1) {
      if (need_separator && after_context_ > 0) {
        out.write("--\n");
      }
    }

    std::string output_line;
    if (!filename.empty()) {
      output_line = filename + ":" + lines[idx] + "\n";
    } else {
      output_line = lines[idx] + "\n";
    }
    out.write(output_line);

    last_printed = idx;
    need_separator = true;
  }
}

int GrepCommand::processInput(Input& in, Output& out) {
  try {
    std::regex regex = buildRegex();
    std::string content = readAllFromInput(in);
    auto lines = splitIntoLines(content);
    outputMatchingLines(lines, regex, out);
    return 0;
  } catch (const std::regex_error& e) {
    std::cerr << "grep: Invalid regular expression: " << e.what() << '\n';
    return 2;
  }
}

int GrepCommand::processFile(const std::string& filename, Output& out) {
  try {
    std::string content = readAllFromFile(filename);
    std::regex regex = buildRegex();
    auto lines = splitIntoLines(content);
    outputMatchingLines(lines, regex, out, files_.size() > 1 ? filename : "");
    return 0;
  } catch (const std::regex_error& e) {
    std::cerr << "grep: Invalid regular expression: " << e.what() << '\n';
    return 2;
  } catch (const std::runtime_error& e) {
    std::cerr << "grep: " << filename << ": No such file or directory\n";
    return 2;
  }
}

int GrepCommand::run(Input& in, Output& out) {
  if (files_.empty()) {
    return processInput(in, out);
  }

  int exit_code = 0;
  for (const auto& file : files_) {
    int result = processFile(file, out);
    if (result != 0) {
      exit_code = result;
    }
  }

  return exit_code;
}

}  // namespace coreutils

