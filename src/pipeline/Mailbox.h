#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <stop_token>
#include <utility>

namespace ae::pipeline {

template <typename TValue>
struct Versioned {
  std::uint64_t seq = 0;
  TValue value;
};

// Single-slot latest-value channel between pipeline stages. publish() replaces
// the slot and bumps the sequence number - values are dropped, never queued,
// because a stale frame is worse than a missing one for automation. Consumers
// pass the last sequence they processed and receive only newer values, so a
// value is consumed at most once per consumer and skipped sequence numbers
// directly measure drops.
template <typename TValue>
class Mailbox {
 public:
  void publish(TValue value) {
    {
      std::scoped_lock lock(mutex_);
      slot_ = Versioned<TValue>{.seq = nextSeq_++, .value = std::move(value)};
    }
    hasNewer_.notify_all();
  }

  // Non-blocking: the newest value when it is newer than lastSeen.
  std::optional<Versioned<TValue>> poll(const std::uint64_t lastSeen) const {
    std::scoped_lock lock(mutex_);
    if (slot_ && slot_->seq > lastSeen) return slot_;
    return std::nullopt;
  }

  // Blocking: sleep until a value newer than lastSeen exists, the timeout
  // expires, or stop is requested. Returns nullopt on timeout or stop.
  std::optional<Versioned<TValue>> awaitNewer(
      const std::uint64_t lastSeen, std::stop_token stopToken,
      const std::chrono::milliseconds timeout
  ) {
    std::unique_lock lock(mutex_);
    hasNewer_.wait_for(lock, stopToken, timeout, [&] {
      return slot_ && slot_->seq > lastSeen;
    });
    if (slot_ && slot_->seq > lastSeen) return slot_;
    return std::nullopt;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable_any hasNewer_;
  std::uint64_t nextSeq_ = 1;
  std::optional<Versioned<TValue>> slot_;
};

}  // namespace ae::pipeline
