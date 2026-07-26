#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "capture/WindowInfo.h"

namespace ae::control {

// UI-to-control requests. Unlike the frame mailbox these must all arrive, so
// this is a real queue, drained once per control loop iteration.

struct LoadScript {
  std::string path;
};

struct UnloadScript {};

struct SetPaused {
  bool paused = false;
};

struct SetTarget {
  std::optional<capture::WindowInfo> target;
};

struct SetRequireFocus {
  bool require = true;
};

using Command =
    std::variant<LoadScript, UnloadScript, SetPaused, SetTarget, SetRequireFocus>;

class CommandQueue {
 public:
  void push(Command command) {
    std::scoped_lock lock(mutex_);
    pending_.push_back(std::move(command));
  }

  std::vector<Command> drain() {
    std::scoped_lock lock(mutex_);
    return std::exchange(pending_, {});
  }

 private:
  std::mutex mutex_;
  std::vector<Command> pending_;
};

}  // namespace ae::control
