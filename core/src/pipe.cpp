#include <memory>
#include <pipe.hpp>

#include <array>
#include <vector>

#include <unistd.h>

namespace coreutils {

namespace {

class PipeInput : public Input {
 public:
  explicit PipeInput(int fd) : fd_(fd) {}
  ~PipeInput() override { close(fd_); }

  PipeInput(const PipeInput&) noexcept = delete;
  PipeInput(PipeInput&&) noexcept = default;
  PipeInput& operator=(const PipeInput&) noexcept = delete;
  PipeInput& operator=(PipeInput&&) noexcept = default;

  std::vector<char> read(size_t size) override {
    std::vector<char> buf(size);
    ssize_t count = ::read(fd_, buf.data(), size);
    if (count == 0) {
      return {};
    }
    buf.resize(count);
    return buf;
  }

 private:
  int fd_;
};

class PipeOutput : public Output {
 public:
  explicit PipeOutput(int fd) : fd_(fd) {}
  ~PipeOutput() override { close(fd_); }

  PipeOutput(const PipeOutput&) noexcept = delete;
  PipeOutput(PipeOutput&&) noexcept = default;
  PipeOutput& operator=(const PipeOutput&) noexcept = delete;
  PipeOutput& operator=(PipeOutput&&) noexcept = default;

  void write(std::vector<char> data) override {
    ::write(fd_, data.data(), data.size());
  }

 private:
  int fd_;
};

}  // namespace

std::pair<std::unique_ptr<Input>, std::unique_ptr<Output>> createPipe() {
  std::array<int, 2> fds{};
  if (pipe(fds.data()) == -1) {
    throw std::runtime_error("Pipe went wrong");
  }

  return {std::make_unique<PipeInput>(fds[0]),
          std::make_unique<PipeOutput>(fds[1])};
}

}  // namespace coreutils
