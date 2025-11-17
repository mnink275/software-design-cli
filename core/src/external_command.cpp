#include <cstdlib>
#include <external_command.hpp>

#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace coreutils {

int ExternalCommand::run(Input& in, Output& out) {
  std::array<int, 2> writePipe{};
  if (pipe(writePipe.data()) == -1) {
    throw std::runtime_error("Pipe went wrong");
  }

  std::array<int, 2> readPipe{};
  if (pipe(readPipe.data()) == -1) {
    throw std::runtime_error("Pipe went wrong");
  }

  pid_t pid = fork();
  if (pid < 0) {
    throw std::runtime_error("Fork went wrong");
  }

  if (pid == 0) {
    close(readPipe[0]);
    close(writePipe[1]);
    int wPipe = readPipe[1];
    int rPipe = writePipe[0];
    dup2(rPipe, STDIN_FILENO);
    dup2(wPipe, STDOUT_FILENO);
    close(rPipe);
    close(wPipe);
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
    close(writePipe[0]);
    close(readPipe[1]);
    int wPipe = writePipe[1];
    int rPipe = readPipe[0];
    child_ = pid;
    std::thread writer([&]() {
      while (true) {
        auto data = in.read(DEFAULT_BLOCK_SIZE);
        if (data.empty()) {
          break;
        }
        write(wPipe, data.data(), data.size());
      }
      close(wPipe);
    });
    std::thread reader([&]() {
      std::array<char, DEFAULT_BLOCK_SIZE> buf{};
      while (true) {
        ssize_t size = read(rPipe, buf.data(), DEFAULT_BLOCK_SIZE);
        if (size <= 0) {
          break;
        }
        out.write({buf.data(), buf.data() + size});
      }
      close(rPipe);
    });
    writer.join();
    reader.join();
    int status = 0;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
  }
}

}  // namespace coreutils
