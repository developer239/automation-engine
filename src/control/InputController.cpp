#include "control/InputController.h"

#include <array>
#include <cmath>

#include <spdlog/spdlog.h>

#include "capture/AppActivation.h"
#include "capture/WindowQueries.h"

namespace ae::control {

namespace {

constexpr auto kKeyNames = std::to_array<std::pair<std::string_view, robot::Key>>({
    {"a", robot::Key::A},
    {"b", robot::Key::B},
    {"c", robot::Key::C},
    {"d", robot::Key::D},
    {"e", robot::Key::E},
    {"f", robot::Key::F},
    {"g", robot::Key::G},
    {"h", robot::Key::H},
    {"i", robot::Key::I},
    {"j", robot::Key::J},
    {"k", robot::Key::K},
    {"l", robot::Key::L},
    {"m", robot::Key::M},
    {"n", robot::Key::N},
    {"o", robot::Key::O},
    {"p", robot::Key::P},
    {"q", robot::Key::Q},
    {"r", robot::Key::R},
    {"s", robot::Key::S},
    {"t", robot::Key::T},
    {"u", robot::Key::U},
    {"v", robot::Key::V},
    {"w", robot::Key::W},
    {"x", robot::Key::X},
    {"y", robot::Key::Y},
    {"z", robot::Key::Z},
    {"0", robot::Key::Digit0},
    {"1", robot::Key::Digit1},
    {"2", robot::Key::Digit2},
    {"3", robot::Key::Digit3},
    {"4", robot::Key::Digit4},
    {"5", robot::Key::Digit5},
    {"6", robot::Key::Digit6},
    {"7", robot::Key::Digit7},
    {"8", robot::Key::Digit8},
    {"9", robot::Key::Digit9},
    {"enter", robot::Key::Enter},
    {"escape", robot::Key::Escape},
    {"backspace", robot::Key::Backspace},
    {"tab", robot::Key::Tab},
    {"space", robot::Key::Space},
    {"up", robot::Key::UpArrow},
    {"down", robot::Key::DownArrow},
    {"left", robot::Key::LeftArrow},
    {"right", robot::Key::RightArrow},
    {"shift", robot::Key::LeftShift},
    {"ctrl", robot::Key::LeftControl},
    {"alt", robot::Key::LeftAlt},
    {"cmd", robot::Key::LeftMeta},
    {"f1", robot::Key::F1},
    {"f2", robot::Key::F2},
    {"f3", robot::Key::F3},
    {"f4", robot::Key::F4},
    {"f5", robot::Key::F5},
    {"f6", robot::Key::F6},
    {"f7", robot::Key::F7},
    {"f8", robot::Key::F8},
    {"f9", robot::Key::F9},
    {"f10", robot::Key::F10},
    {"f11", robot::Key::F11},
    {"f12", robot::Key::F12},
});

InputResult fromRobotError(const robot::Error& error) {
  return InputResult::refused("input_failed", error.message);
}

}  // namespace

std::optional<robot::Key> keyFromName(const std::string_view name) {
  for (const auto& [candidate, key] : kKeyNames) {
    if (candidate == name) return key;
  }
  return std::nullopt;
}

std::optional<robot::MouseButton> mouseButtonFromName(
    const std::string_view name
) {
  if (name == "left") return robot::MouseButton::Left;
  if (name == "right") return robot::MouseButton::Right;
  if (name == "middle") return robot::MouseButton::Middle;
  return std::nullopt;
}

std::span<const std::pair<std::string_view, robot::Key>> keyNameTable() {
  return kKeyNames;
}

void InputController::setTarget(std::optional<capture::WindowInfo> target) {
  releaseAll();
  target_ = std::move(target);
}

bool InputController::targetFocused() const {
  if (!target_) return false;
  return capture::frontmostWindowId() == target_->windowId;
}

std::optional<InputResult> InputController::gate() const {
  if (!tick_) {
    return InputResult::refused(
        "outside_tick", "input is only available inside onFrame"
    );
  }
  if (!target_) {
    return InputResult::refused("no_target", "no window is being captured");
  }
  if (tick_->windowId != target_->windowId) {
    // A frame from the previous stream can survive a capture switch inside
    // the mailbox; its coordinates mean nothing against the new target.
    return InputResult::refused(
        "frame_target_mismatch",
        "this frame was captured from a different window than the current "
        "target"
    );
  }

  const auto frameAge = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - tick_->capturedAt
  );
  if (frameAge > staleFrameLimit_) {
    return InputResult::refused(
        "stale_frame",
        std::format(
            "frame is {} ms old (limit {} ms)", frameAge.count(),
            staleFrameLimit_.count()
        )
    );
  }

  if (requireFocus_ && !targetFocused()) {
    return InputResult::refused(
        "target_not_focused", "target window is not frontmost"
    );
  }

  return std::nullopt;
}

std::optional<InputResult> InputController::toGlobal(
    const double x, const double y, robot::LogicalPoint& out
) const {
  if (!std::isfinite(x) || !std::isfinite(y) || x < 0 || y < 0 ||
      x >= tick_->frameWidth || y >= tick_->frameHeight) {
    return InputResult::refused(
        "out_of_bounds",
        std::format(
            "({}, {}) is outside the {}x{} frame", x, y, tick_->frameWidth,
            tick_->frameHeight
        )
    );
  }

  if (tick_->scaleFactor <= 0.0 || tick_->contentScale <= 0.0 ||
      tick_->contentWidth <= 0.0 || tick_->contentHeight <= 0.0) {
    return InputResult::refused(
        "missing_scale", "frame carries no scale metadata; cannot map "
                         "capture pixels to desktop coordinates"
    );
  }

  // Buffer pixels -> surface points -> content-local points (the content can
  // sit away from the surface origin: shadows, padding, resize letterboxing)
  // -> window points.
  const double contentPointX = x / tick_->scaleFactor - tick_->contentX;
  const double contentPointY = y / tick_->scaleFactor - tick_->contentY;
  if (contentPointX < 0 || contentPointX >= tick_->contentWidth ||
      contentPointY < 0 || contentPointY >= tick_->contentHeight) {
    return InputResult::refused(
        "out_of_bounds",
        std::format(
            "({}, {}) lies outside the window content area", x, y
        )
    );
  }

  const auto frame = capture::windowFrame(target_->windowId);
  if (!frame) {
    return InputResult::refused("window_gone", "target window disappeared");
  }

  out = robot::LogicalPoint{
      .x = frame->origin.x + contentPointX / tick_->contentScale,
      .y = frame->origin.y + contentPointY / tick_->contentScale,
  };
  return std::nullopt;
}

InputResult InputController::moveMouse(
    const double x, const double y, const bool smooth
) {
  if (auto refusal = gate()) return *refusal;
  robot::LogicalPoint point;
  if (auto refusal = toGlobal(x, y, point)) return *refusal;

  const auto result = smooth ? session_.mouse().moveSmooth(point)
                             : session_.mouse().move(point);
  if (!result) return fromRobotError(result.error());
  return InputResult::success();
}

InputResult InputController::click(
    const double x, const double y, const std::string_view buttonName
) {
  if (auto refusal = gate()) return *refusal;
  const auto button = mouseButtonFromName(buttonName);
  if (!button) {
    return InputResult::refused(
        "unknown_button", std::format("unknown mouse button '{}'", buttonName)
    );
  }
  robot::LogicalPoint point;
  if (auto refusal = toGlobal(x, y, point)) return *refusal;

  if (const auto moved = session_.mouse().move(point); !moved) {
    return fromRobotError(moved.error());
  }
  if (const auto clicked = session_.mouse().click(*button); !clicked) {
    return fromRobotError(clicked.error());
  }
  return InputResult::success();
}

InputResult InputController::pressButton(const std::string_view buttonName) {
  if (auto refusal = gate()) return *refusal;
  const auto button = mouseButtonFromName(buttonName);
  if (!button) {
    return InputResult::refused(
        "unknown_button", std::format("unknown mouse button '{}'", buttonName)
    );
  }
  if (heldButtons_.contains(*button)) return InputResult::success();

  if (const auto result = session_.mouse().press(*button); !result) {
    return fromRobotError(result.error());
  }
  heldButtons_.insert(*button);
  return InputResult::success();
}

InputResult InputController::releaseButton(const std::string_view buttonName) {
  if (auto refusal = gate()) return *refusal;
  const auto button = mouseButtonFromName(buttonName);
  if (!button) {
    return InputResult::refused(
        "unknown_button", std::format("unknown mouse button '{}'", buttonName)
    );
  }
  if (!heldButtons_.contains(*button)) return InputResult::success();

  if (const auto result = session_.mouse().release(*button); !result) {
    return fromRobotError(result.error());
  }
  heldButtons_.erase(*button);
  return InputResult::success();
}

InputResult InputController::pressKey(const std::string_view keyName) {
  if (auto refusal = gate()) return *refusal;
  const auto key = keyFromName(keyName);
  if (!key) {
    return InputResult::refused(
        "unknown_key", std::format("unknown key '{}'", keyName)
    );
  }
  if (heldKeys_.contains(*key)) return InputResult::success();

  if (const auto result = session_.keyboard().press(*key); !result) {
    return fromRobotError(result.error());
  }
  heldKeys_.insert(*key);
  return InputResult::success();
}

InputResult InputController::releaseKey(const std::string_view keyName) {
  if (auto refusal = gate()) return *refusal;
  const auto key = keyFromName(keyName);
  if (!key) {
    return InputResult::refused(
        "unknown_key", std::format("unknown key '{}'", keyName)
    );
  }
  if (!heldKeys_.contains(*key)) return InputResult::success();

  if (const auto result = session_.keyboard().release(*key); !result) {
    return fromRobotError(result.error());
  }
  heldKeys_.erase(*key);
  return InputResult::success();
}

InputResult InputController::tapKey(const std::string_view keyName) {
  if (auto refusal = gate()) return *refusal;
  const auto key = keyFromName(keyName);
  if (!key) {
    return InputResult::refused(
        "unknown_key", std::format("unknown key '{}'", keyName)
    );
  }

  if (const auto result = session_.keyboard().tap(*key); !result) {
    return fromRobotError(result.error());
  }
  return InputResult::success();
}

InputResult InputController::typeText(const std::string_view text) {
  if (auto refusal = gate()) return *refusal;

  if (const auto result = session_.keyboard().typeText(text); !result) {
    return fromRobotError(result.error());
  }
  return InputResult::success();
}

InputResult InputController::focusTarget() {
  if (!target_) {
    return InputResult::refused("no_target", "no window is being captured");
  }

  if (!capture::activateApplication(target_->ownerPid)) {
    return InputResult::refused(
        "activation_failed", "target application refused activation or is gone"
    );
  }
  return InputResult::success();
}

void InputController::releaseAll() {
  if (heldKeys_.empty() && heldButtons_.empty()) return;

  for (const auto key : heldKeys_) {
    if (const auto result = session_.keyboard().release(key); !result) {
      spdlog::warn(
          "[Input] failed to release key {}: {}", toString(key),
          result.error().message
      );
    }
  }
  for (const auto button : heldButtons_) {
    if (const auto result = session_.mouse().release(button); !result) {
      spdlog::warn(
          "[Input] failed to release button {}: {}", toString(button),
          result.error().message
      );
    }
  }

  spdlog::info(
      "[Input] released {} held keys, {} held buttons", heldKeys_.size(),
      heldButtons_.size()
  );
  heldKeys_.clear();
  heldButtons_.clear();
}

}  // namespace ae::control
