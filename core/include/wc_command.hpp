#pragma once

#include <command.hpp>

#include <string>
#include <vector>

namespace coreutils {

class WcCommand final : public Command {
 public:
  explicit WcCommand(std::vector<std::string> files)
      : files_(std::move(files)) {}

  int run(Input& in, Output& out) override;

 private:
  struct FileStats {
    size_t lines{};
    size_t words{};
    size_t bytes{};
  };

  [[nodiscard]] FileStats collectStats(const std::string& file) const;

 private:
  std::vector<std::string> files_;
};

}  // namespace coreutils

