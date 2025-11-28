#pragma once

#include <output.hpp>
#include <pipe.hpp>

#include <optional>
#include <thread>

namespace coreutils {

class TextOutput final : public Output {
 public:
  explicit TextOutput() {
    auto [in, out] = createPipe();
    out_ = std::move(out);  // NOLINT
    thread_.emplace([in = std::move(in), this]() { str_ = in->readString(); });
  }

  ~TextOutput() override {
    if (out_) {
      out_.reset();
      thread_->join();
    }
  }
  TextOutput(const TextOutput&) noexcept = delete;
  TextOutput(TextOutput&&) noexcept = delete;
  TextOutput& operator=(const TextOutput&) noexcept = delete;
  TextOutput& operator=(TextOutput&&) noexcept = delete;

  [[nodiscard]] int fd() const override { return out_->fd(); }

  [[nodiscard]] const std::string& read() const& {
    if (out_) {
      out_.reset();
      thread_->join();
    }
    return str_;
  }
  [[nodiscard]] std::string&& read() && {
    if (out_) {
      out_.reset();
      thread_->join();
    }
    return std::move(str_);
  }

 private:
  mutable std::unique_ptr<Output> out_{};
  mutable std::optional<std::thread> thread_;
  std::string str_;
};

}  // namespace coreutils
