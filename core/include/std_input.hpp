#pragma once

#include <input.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

namespace coreutils {

class StdInput final : public Input {
 public:
  std::vector<char> read(size_t size) override {
    if (size == 0 || !std::cin.good()) {
      return {};
    }

    std::vector<char> res;
    res.reserve(size);

    char c{};
    while (res.size() < size && std::cin.get(c)) {
      res.push_back(c);
      if (c == '\n') break;
    }

    return res;
  }
};

}  // namespace coreutils
