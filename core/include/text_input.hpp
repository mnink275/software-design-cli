#pragma once

#include <input.hpp>
#include <pipe.hpp>

#include <optional>
#include <thread>

namespace coreutils {

class TextInput final : public Input {
 public:
  explicit TextInput(std::string str) {
    auto [in, out] = createPipe();
    in_ = std::move(in);  // NOLINT
    thread_.emplace(
        [str = std::move(str), out = std::move(out)]() { out->write(str); });
  }
  ~TextInput() override {
    in_.reset();
    thread_->join();
  }
  TextInput(const TextInput&) noexcept = delete;
  TextInput(TextInput&&) noexcept = delete;
  TextInput& operator=(const TextInput&) noexcept = delete;
  TextInput& operator=(TextInput&&) noexcept = delete;

  [[nodiscard]] int fd() const override { return in_->fd(); }

 private:
  std::unique_ptr<Input> in_{};
  std::optional<std::thread> thread_;
};

}  // namespace coreutils
