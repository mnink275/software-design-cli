#include <executor.hpp>

#include <cassert>
#include <string>
#include <vector>

#include <command.hpp>

namespace coreutils {

int Executor::runCommands(std::vector<CommandPtr> cmds, Input& in,
                          Output& out) {
  // run 1 command
  assert(cmds.size() == 1);
  return cmds[0]->run(in, out);
}

}  // namespace coreutils
