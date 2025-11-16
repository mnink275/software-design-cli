#pragma once

#include <output.hpp>

#include <string>
#include <vector>

namespace coreutils {

class TextOutput : public Output {
 public:
  void write(std::vector<char> data) override {
    str_.insert(str_.end(), data.begin(), data.end());
  }
  [[nodiscard]] const std::string& read() const& { return str_; }
  std::string read() && { return std::move(str_); }

 private:
  std::string str_;
};

}  // namespace coreutils
