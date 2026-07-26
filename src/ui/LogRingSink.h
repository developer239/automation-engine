#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include <spdlog/sinks/base_sink.h>

namespace ae::ui {

// spdlog sink retaining the most recent messages for the in-app log window.
// The UI polls version() and re-copies the buffer only when it changed.
class LogRingSink final : public spdlog::sinks::base_sink<std::mutex> {
 public:
  struct Entry {
    spdlog::level::level_enum level = spdlog::level::info;
    std::string text;
  };

  explicit LogRingSink(const std::size_t capacity = 2000)
      : capacity_(capacity) {}

  [[nodiscard]] std::uint64_t version() {
    std::scoped_lock lock(base_sink<std::mutex>::mutex_);
    return version_;
  }

  [[nodiscard]] std::vector<Entry> snapshot() {
    std::scoped_lock lock(base_sink<std::mutex>::mutex_);
    return {entries_.begin(), entries_.end()};
  }

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    base_sink<std::mutex>::formatter_->format(msg, formatted);
    entries_.push_back(Entry{
        .level = msg.level,
        .text = fmt::to_string(formatted),
    });
    if (entries_.size() > capacity_) entries_.pop_front();
    ++version_;
  }

  void flush_() override {}

 private:
  std::size_t capacity_;
  std::deque<Entry> entries_;
  std::uint64_t version_ = 0;
};

}  // namespace ae::ui
