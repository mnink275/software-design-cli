#include "ls_command.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace coreutils {

LsCommand::LsCommand(std::vector<std::string> args)
    : paths_(std::move(args)) {}

int LsCommand::run(Input&, Output& out) {
  if (paths_.size() > 1) {
    std::cerr << "ls: too many  argumements\n";
    return 1;
  }
std::filesystem::path target =
    paths_.empty() ? std::filesystem::current_path() : std::filesystem::path(paths_[0]);

std::error_code ec;

if (!std::filesystem::exists(target, ec)) {
  std::cerr << "ls: cannot access '" << target.string()
            << "': No such file or directory\n";
  return 1;
}


  if (std::filesystem::is_regular_file(target, ec)) {
    out.write(target.filename().string() + "\n");
    return 0;
  }

  if (std::filesystem::is_directory(target, ec)) {
    std::vector<std::string> entries;

    for (const auto& entry :
         std::filesystem::directory_iterator(target, ec)) {
      entries.push_back(entry.path().filename().string());
    }

    std::sort(entries.begin(), entries.end());

    for (const auto& name : entries) {
      out.write(name + "\n");
    }

    return 0;
  }

  std::cerr << "ls: cannot access '" << target.string() << "'\n";
  return 1;
}

}  // namespace coreutils
