#include <executor.hpp>

#include <cassert>
#include <vector>

#include <command.hpp>
#include <pipe.hpp>

namespace coreutils {

int Executor::runCommands(std::vector<CommandPtr> cmds, Input& in,
                          Output& out) {
  assert(!cmds.empty());

  if (cmds.size() == 1) {
    return cmds[0]->run(in, out);
  }

  std::unique_ptr<Input> current_input;
  Input* in_ptr = &in;

  int exit_code = 0;

  for (size_t i = 0; i < cmds.size(); ++i) {
    const bool is_last_cmd = (i + 1 == cmds.size());

    if (!is_last_cmd) {
      // Пайп между cmd[i] и cmd[i+1]
      auto [pipe_in, pipe_out] = createPipe();
      exit_code = cmds[i]->run(*in_ptr, *pipe_out);

      // cmd[i+1] будет читать из pipe_in
      current_input = std::move(pipe_in);
      in_ptr = current_input.get();
    } else {
      // Последняя команда пишет в out
      exit_code = cmds[i]->run(*in_ptr, out);
    }
  }

  return exit_code;
}

}  // namespace coreutils
