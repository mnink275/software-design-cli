#include <input.hpp>

#include <array>
#include <stdexcept>

#include <unistd.h>

namespace coreutils {

size_t Input::read(char* data, size_t size) const {
  auto res = ::read(fd(), data, size);
  if (res == -1) {
    throw std::runtime_error("Read failed");
  }

  return res;
}

std::vector<char> Input::readVector(size_t size) const {
  std::vector<char> res(size);
  res.resize(read(res.data(), size));
  return res;
}

std::vector<char> Input::readVector() const {
  std::vector<char> res;
  while (true) {
    std::array<char, DEFAULT_BLOCK_SIZE> buf{};
    auto size = read(buf.data(), DEFAULT_BLOCK_SIZE);
    if (size == 0) {
        return res;
    }
    res.insert(res.end(), buf.begin(), buf.begin() + size);
  }
}

std::string Input::readString(size_t size) const {
  std::string res(size, 0);
  res.resize(read(res.data(), size));
  return res;
}

std::string Input::readString() const {
  std::string res;
  while (true) {
    std::array<char, DEFAULT_BLOCK_SIZE> buf{};
    auto size = read(buf.data(), DEFAULT_BLOCK_SIZE);
    if (size == 0) {
        return res;
    }
    res.insert(res.end(), buf.begin(), buf.begin() + size);
  }
}

void Input::setStdin() const {
  dup2(fd(), STDIN_FILENO);
}

}  // namespace coreutils
