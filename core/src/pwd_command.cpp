#include <pwd_command.hpp>

#include <filesystem>
#include <string>

namespace coreutils {

int PwdCommand::run(Input&, Output& out) {
  auto path = std::filesystem::current_path().string();
  path.push_back('\n');
  out.write({path.begin(), path.end()});
  return 0;
}

}  // namespace coreutils

