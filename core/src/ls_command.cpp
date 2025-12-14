#include <ls_command.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace coreutils {

namespace {

std::filesystem::path ExpandHome(const std::string& path) {
  if (path.empty() || path[0] != '~') {
    return std::filesystem::path(path);
  }

  const char* home = std::getenv("HOME");
  if (home == nullptr) {
    throw std::runtime_error("HOME is not set");
  }

  if (path.size() == 1) {
    return std::filesystem::path(home);
  }

  if (path[1] == '/') {
    return std::filesystem::path(home) / path.substr(2);
  }

  return std::filesystem::path(path);
}

}  // namespace

LsCommand::LsCommand(std::vector<std::string> args) {
  if (args.size() > 1) {
    throw std::invalid_argument("ls accepts at most one argument");
  }

  if (!args.empty()) {
    target_ = std::move(args[0]);
  }
}

int LsCommand::run(Input&, Output& out) {
  try {
    std::filesystem::path target =
        target_ ? ExpandHome(*target_) : std::filesystem::current_path();

    if (!std::filesystem::exists(target)) {
      std::cerr << "ls: cannot access '" << target.string()
                << "': No such file or directory\n";
      return 1;
    }

    if (!std::filesystem::is_directory(target)) {
      auto value = target.string();
      value.push_back('\n');
      out.write(value);
      return 0;
    }

    std::vector<std::string> entries;
    for (const auto& entry : std::filesystem::directory_iterator(target)) {
      entries.push_back(entry.path().filename().string());
    }
    std::sort(entries.begin(), entries.end());

    for (auto& entry : entries) {
      entry.push_back('\n');
      out.write(entry);
    }

    return 0;
  } catch (const std::filesystem::filesystem_error& err) {
    std::cerr << "ls: " << err.what() << '\n';
  } catch (const std::runtime_error& err) {
    std::cerr << "ls: " << err.what() << '\n';
  }

  return 1;
}

}  // namespace coreutils
