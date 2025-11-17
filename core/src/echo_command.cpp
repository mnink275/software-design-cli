#include <echo_command.hpp>

#include <sstream>

namespace coreutils {

int EchoCommand::run(Input&, Output& out) {
  std::ostringstream oss;

  for (size_t i = 0; i < args_.size(); ++i) {
    if (i > 0) {
      oss << ' ';
    }
    oss << args_[i];
  }

  oss << '\n';
  auto str = oss.str();
  out.write({str.begin(), str.end()});
  return 0;
}

}  // namespace coreutils

