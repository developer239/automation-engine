#pragma once

#include <cstdint>
#include <string>

#include "pipeline/Mailbox.h"

namespace ae::pipeline {

// Per-tick observability snapshot published by the control thread for the UI.
// "End-to-end latency" alone cannot identify a slow stage, so each stage's
// contribution is reported separately.
struct ControlStats {
  std::uint64_t frameSeq = 0;
  std::uint64_t droppedFrames = 0;
  double frameAgeAtTickMs = 0.0;
  double tickDurationMs = 0.0;
  bool scriptLoaded = false;
  bool scriptPaused = false;
  bool targetFocused = false;
  std::string scriptPath;
  std::string lastScriptError;
};

using StatsMailbox = Mailbox<ControlStats>;

}  // namespace ae::pipeline
