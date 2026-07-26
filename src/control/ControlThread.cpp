#include "control/ControlThread.h"

#include <chrono>
#include <optional>
#include <variant>

#include <spdlog/spdlog.h>

namespace ae::control {

namespace {

using Clock = std::chrono::steady_clock;

double toMs(const Clock::duration duration) {
  return std::chrono::duration<double, std::milli>(duration).count();
}

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

}  // namespace

void ControlThread::start() {
  thread_ = std::jthread([this](const std::stop_token& token) { run(token); });
}

void ControlThread::stop() {
  if (thread_.joinable()) {
    thread_.request_stop();
    thread_.join();
  }
}

void ControlThread::run(const std::stop_token& stopToken) {
  spdlog::info("[Control] thread started");
  std::optional<Clock::time_point> previousCaptureAt;

  while (!stopToken.stop_requested()) {
    for (auto& command : commands_.drain()) {
      apply(std::move(command));
    }

    const auto frame = frames_.awaitNewer(
        lastSeq_, stopToken, std::chrono::milliseconds{50}
    );
    if (!frame) {
      // Frames stopped arriving (stream died, window frozen or gone). Focus
      // transitions and held input must still reconcile - a key pressed
      // during the last frame must not stay down until shutdown.
      const bool focused = input_.targetFocused();
      if (wasFocused_ && !focused) input_.releaseAll();
      wasFocused_ = focused;
      if (lastFrameAt_ &&
          Clock::now() - *lastFrameAt_ > kStallReleaseThreshold) {
        input_.releaseAll();
      }
      publishStats();
      continue;
    }
    lastFrameAt_ = Clock::now();

    droppedFrames_ += frame->seq - lastSeq_ - 1;
    lastSeq_ = frame->seq;
    const capture::CapturedFrame& captured = frame->value;

    const auto tickStart = Clock::now();
    lastFrameAgeMs_ = toMs(tickStart - captured.capturedAt);
    const double deltaMs =
        previousCaptureAt ? toMs(captured.capturedAt - *previousCaptureAt) : 0.0;
    previousCaptureAt = captured.capturedAt;

    const bool focused = input_.targetFocused();
    if (wasFocused_ && !focused) input_.releaseAll();
    wasFocused_ = focused;

    if (script_.loaded() && !paused_) {
      input_.beginTick(InputController::TickContext{
          .windowId = captured.windowId,
          .frameWidth = captured.width,
          .frameHeight = captured.height,
          .scaleFactor = captured.scaleFactor,
          .contentScale = captured.contentScale,
          .contentX = captured.contentX,
          .contentY = captured.contentY,
          .contentWidth = captured.contentWidth,
          .contentHeight = captured.contentHeight,
          .capturedAt = captured.capturedAt,
      });
      auto overlay = script_.onFrame(scripting::ScriptRuntime::FrameView{
          .seq = lastSeq_,
          .width = captured.width,
          .height = captured.height,
          .ageMs = lastFrameAgeMs_,
          .deltaMs = deltaMs,
          .focused = focused,
      });
      input_.endTick();
      lastTickDurationMs_ = toMs(Clock::now() - tickStart);

      if (overlay) {
        lastError_.clear();
        overlays_.publish(pipeline::OverlaySnapshot{
            .frameSeq = lastSeq_, .commands = std::move(*overlay)});
      } else {
        // A failing script must not keep steering: release input and pause
        // until the user reloads or unpauses explicitly.
        lastError_ = overlay.error().message;
        spdlog::error("[Control] {} (script paused)", lastError_);
        input_.releaseAll();
        paused_ = true;
      }
    }

    publishStats();
  }

  input_.releaseAll();
  spdlog::info("[Control] thread stopped");
}

void ControlThread::apply(Command command) {
  std::visit(
      Overloaded{
          [this](LoadScript& load) {
            input_.releaseAll();
            if (auto result = script_.load(load.path); !result) {
              lastError_ = result.error().message;
              spdlog::error("[Control] {}", lastError_);
            } else {
              lastError_.clear();
              paused_ = false;
            }
          },
          [this](const UnloadScript&) {
            input_.releaseAll();
            script_.unload();
            lastError_.clear();
          },
          [this](const SetPaused& pause) {
            if (pause.paused && !paused_) input_.releaseAll();
            paused_ = pause.paused;
          },
          [this](SetTarget& target) {
            input_.setTarget(std::move(target.target));
          },
          [this](const SetRequireFocus& focus) {
            input_.setRequireFocus(focus.require);
          },
      },
      command
  );
}

void ControlThread::publishStats() {
  stats_.publish(pipeline::ControlStats{
      .frameSeq = lastSeq_,
      .droppedFrames = droppedFrames_,
      .frameAgeAtTickMs = lastFrameAgeMs_,
      .tickDurationMs = lastTickDurationMs_,
      .scriptLoaded = script_.loaded(),
      .scriptPaused = paused_,
      .targetFocused = input_.targetFocused(),
      .scriptPath = script_.path(),
      .lastScriptError = lastError_,
  });
}

}  // namespace ae::control
