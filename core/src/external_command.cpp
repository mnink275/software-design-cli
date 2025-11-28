#include <cstdlib>
#include <external_command.hpp>

#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

namespace coreutils {

int ExternalCommand::run(Input& in, Output& out) {
  pid_t pid = fork();
  if (pid < 0) {
    throw std::runtime_error("Fork went wrong");
  }

  if (pid == 0) {
    in.setStdin();
    out.setStdout();
    std::vector<char*> argvs;
    argvs.reserve(args_.size() + 2);
    argvs.push_back(command_.data());
    for (auto& arg : args_) {
      argvs.push_back(arg.data());
    }
    argvs.push_back(nullptr);
    execvp(command_.data(), argvs.data());
    std::cerr << command_ << ": command not found\n";
    _exit(127);
  } else {
    child_ = pid;
    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

}  // namespace coreutils
