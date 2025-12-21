#include <cd_command.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

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

std::filesystem::path ResolveTarget(
    const std::optional<std::string>& argument) {
  if (!argument || argument->empty()) {
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
      throw std::runtime_error("HOME is not set");
    }
    return std::filesystem::path(home);
  }

  return ExpandHome(*argument);
}

}  // namespace

CdCommand::CdCommand(std::vector<std::string> args) {
  if (args.size() > 1) {
    throw std::invalid_argument("cd accepts at most one argument");
  }

  if (!args.empty()) {
    target_ = std::move(args[0]);
  }
}

int CdCommand::run(Input&, Output&) {
  try {
    auto target = ResolveTarget(target_);
    if (!std::filesystem::exists(target)) {
      std::cerr << "cd: " << target.string() << ": No such file or directory\n";
      return 1;
    }

    if (!std::filesystem::is_directory(target)) {
      std::cerr << "cd: " << target.string() << ": Not a directory\n";
      return 1;
    }

    std::filesystem::current_path(target);
    return 0;
  } catch (const std::filesystem::filesystem_error& err) {
    std::cerr << "cd: " << err.what() << '\n';
  } catch (const std::runtime_error& err) {
    std::cerr << "cd: " << err.what() << '\n';
  }

  return 1;
}

}  // namespace coreutils
