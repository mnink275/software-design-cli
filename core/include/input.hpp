#pragma once

#include <string>
#include <vector>

namespace coreutils {

constexpr size_t DEFAULT_BLOCK_SIZE = 4096;

class Input {
 public:
  Input() = default;
  virtual ~Input() = default;
  Input(const Input&) noexcept = delete;
  Input(Input&&) noexcept = default;
  Input& operator=(const Input&) noexcept = delete;
  Input& operator=(Input&&) noexcept = default;

  size_t read(char* data, size_t size) const;
  [[nodiscard]] std::vector<char> readVector(size_t size) const;
  [[nodiscard]] std::vector<char> readVector() const;
  [[nodiscard]] std::string readString(size_t size) const;
  [[nodiscard]] std::string readString() const;
  void setStdin() const;

  [[nodiscard]] virtual int fd() const = 0;
};

}  // namespace coreutils
