#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include "capture/WindowInfo.h"
#include "robot/Session.h"

namespace ae::control {

// Result of one script-issued input action, shaped for Lua's
// `true` / `nil, {code, message}` convention. Refusals are results the script
// can react to, never silent no-ops.
struct InputResult {
  bool ok = false;
  std::string code;
  std::string message;

  static InputResult success() { return {.ok = true}; }

  static InputResult refused(std::string code, std::string message) {
    return {
        .ok = false, .code = std::move(code), .message = std::move(message)};
  }
};

// Serialized wrapper over robot-cpp injection, owned and driven exclusively by
// the control thread (no internal locking - single-thread by contract).
//
// Every coordinate-based action re-reads the target window's current bounds
// and the frontmost application immediately before injecting, because both
// are stale the moment they are observed anywhere else. Held keys and buttons
// are tracked so releaseAll() can reconcile input state on the transitions
// where a held key is dangerous: pause, script reload/error, focus loss,
// target change, shutdown.
class InputController {
 public:
  // Metadata of the frame the current script tick runs against. Input is only
  // legal inside a tick (between beginTick and endTick).
  struct TickContext {
    std::uint32_t windowId = 0;
    std::int32_t frameWidth = 0;
    std::int32_t frameHeight = 0;
    double scaleFactor = 0.0;
    double contentScale = 0.0;
    double contentX = 0.0;
    double contentY = 0.0;
    double contentWidth = 0.0;
    double contentHeight = 0.0;
    std::chrono::steady_clock::time_point capturedAt;
  };

  explicit InputController(robot::Session& session) : session_(session) {}

  void setTarget(std::optional<capture::WindowInfo> target);
  void setRequireFocus(const bool require) { requireFocus_ = require; }
  void setStaleFrameLimit(const std::chrono::milliseconds limit) {
    staleFrameLimit_ = limit;
  }

  [[nodiscard]] bool hasTarget() const { return target_.has_value(); }
  [[nodiscard]] bool targetFocused() const;

  void beginTick(const TickContext& context) { tick_ = context; }
  void endTick() { tick_.reset(); }

  // Coordinates are capture pixel space; the transform to global logical
  // desktop coordinates goes through the frame's scale metadata and the
  // window's current bounds.
  InputResult moveMouse(double x, double y, bool smooth);
  InputResult click(double x, double y, std::string_view buttonName);
  InputResult pressButton(std::string_view buttonName);
  InputResult releaseButton(std::string_view buttonName);
  InputResult pressKey(std::string_view keyName);
  InputResult releaseKey(std::string_view keyName);
  InputResult tapKey(std::string_view keyName);
  InputResult typeText(std::string_view text);

  // Brings the target's application to the foreground.
  InputResult focusTarget();

  void releaseAll();

 private:
  // Common preconditions for injection: active tick, target set, frame fresh,
  // target focused. Returns the refusal when one fails.
  std::optional<InputResult> gate() const;
  std::optional<InputResult> toGlobal(double x, double y, robot::LogicalPoint& out) const;

  robot::Session& session_;
  std::optional<capture::WindowInfo> target_;
  bool requireFocus_ = true;
  std::chrono::milliseconds staleFrameLimit_{250};
  std::optional<TickContext> tick_;
  std::set<robot::Key> heldKeys_;
  std::set<robot::MouseButton> heldButtons_;
};

[[nodiscard]] std::optional<robot::Key> keyFromName(std::string_view name);
[[nodiscard]] std::optional<robot::MouseButton> mouseButtonFromName(
    std::string_view name
);

// Canonical script-facing key names, used to generate the Lua `keys` constant
// table and to validate key arguments - one table, two consumers.
[[nodiscard]] std::span<const std::pair<std::string_view, robot::Key>>
keyNameTable();

}  // namespace ae::control
