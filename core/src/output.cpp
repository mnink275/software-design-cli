#include <output.hpp>

#include <stdexcept>

#include <unistd.h>

namespace coreutils {

void Output::write(const char* data, size_t size) const {
  size_t written = 0;
  while (written != size) {
    auto res = ::write(fd(), data + written, size - written);  // NOLINT
    if (res == -1) {
      throw std::runtime_error("Write failed");
    }
    written += res;
  }
}

void Output::write(const std::vector<char>& data) const {
  write(data.data(), data.size());
}

void Output::write(const std::string& data) const {
  write(data.data(), data.size());
}

void Output::setStdout() const { dup2(fd(), STDOUT_FILENO); }

}  // namespace coreutils
