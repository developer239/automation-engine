#pragma once

#include <chrono>
#include <optional>
#include <thread>

#include "capture/WindowCapture.h"
#include "control/Commands.h"
#include "control/InputController.h"
#include "pipeline/Mailbox.h"
#include "pipeline/Overlay.h"
#include "pipeline/Stats.h"
#include "scripting/ScriptRuntime.h"

namespace ae::control {

// The control stage: exclusively owns the Lua runtime and all input
// injection. Sleeps on the frame mailbox, runs the script's onFrame once per
// new frame sequence number, and publishes overlay + stats snapshots for the
// UI thread to observe. UI requests arrive through the command queue.
//
// Safety transitions (releasing all held input) happen here: pause, script
// load/unload/error, target change, focus loss, shutdown.
class ControlThread {
 public:
  ControlThread(
      capture::FrameMailbox& frames, pipeline::OverlayMailbox& overlays,
      pipeline::StatsMailbox& stats, InputController& input,
      scripting::ScriptRuntime& script
  )
      : frames_(frames),
        overlays_(overlays),
        stats_(stats),
        input_(input),
        script_(script) {}

  ~ControlThread() { stop(); }
  ControlThread(const ControlThread&) = delete;
  ControlThread& operator=(const ControlThread&) = delete;
  ControlThread(ControlThread&&) = delete;
  ControlThread& operator=(ControlThread&&) = delete;

  void start();
  void stop();

  [[nodiscard]] CommandQueue& commands() { return commands_; }

 private:
  void run(const std::stop_token& stopToken);
  void apply(Command command);
  void publishStats();

  capture::FrameMailbox& frames_;
  pipeline::OverlayMailbox& overlays_;
  pipeline::StatsMailbox& stats_;
  InputController& input_;
  scripting::ScriptRuntime& script_;
  CommandQueue commands_;
  std::jthread thread_;

  // When frames stop arriving for this long, held input is released - a
  // dead stream must not leave a key down.
  static constexpr std::chrono::milliseconds kStallReleaseThreshold{250};

  // Control-thread-local state (no locking: only run() touches it).
  std::uint64_t lastSeq_ = 0;
  std::uint64_t droppedFrames_ = 0;
  std::optional<std::chrono::steady_clock::time_point> lastFrameAt_;
  double lastFrameAgeMs_ = 0.0;
  double lastTickDurationMs_ = 0.0;
  bool paused_ = false;
  bool wasFocused_ = false;
  std::string lastError_;
};

}  // namespace ae::control
