#pragma once

#include <string>
#include <vector>

namespace coreutils {

class Output {
 public:
  Output() = default;
  virtual ~Output() = default;
  Output(const Output&) noexcept = delete;
  Output(Output&&) noexcept = default;
  Output& operator=(const Output&) noexcept = delete;
  Output& operator=(Output&&) noexcept = default;

  void write(const char* data, size_t size) const;
  void write(const std::vector<char>& data) const;
  void write(const std::string& data) const;
  void setStdout() const;

  [[nodiscard]] virtual int fd() const = 0;
};

}  // namespace coreutils
