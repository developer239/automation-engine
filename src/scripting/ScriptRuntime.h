#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <sol/sol.hpp>

#include "common/Error.h"
#include "control/InputController.h"
#include "pipeline/Overlay.h"

namespace ae::scripting {

// Owns the Lua state and the script-facing API. Lives on the control thread
// exclusively - sol::state is not thread-safe and nothing else may touch it.
//
// Script contract: the file defines `main = { onFrame = function(frame) end }`.
// onFrame runs once per new capture frame. Overlay and input calls are only
// legal during that call; a tick that errors publishes nothing.
class ScriptRuntime {
 public:
  struct FrameView {
    std::uint64_t seq = 0;
    std::int32_t width = 0;
    std::int32_t height = 0;
    double ageMs = 0.0;    // capture-to-tick-start latency
    double deltaMs = 0.0;  // capture-timestamp gap to the previous tick
    bool focused = false;
  };

  explicit ScriptRuntime(
      control::InputController& input,
      std::chrono::milliseconds tickBudget = std::chrono::milliseconds{100}
  )
      : input_(input), tickBudget_(tickBudget) {}

  // Loads (or replaces) the script from a fresh Lua state, so a reload never
  // inherits globals from the previous run.
  std::expected<void, Error> load(const std::string& path);
  void unload();

  [[nodiscard]] bool loaded() const { return lua_.has_value(); }
  [[nodiscard]] const std::string& path() const { return path_; }

  // Runs main.onFrame. On success returns the tick's overlay command buffer;
  // on failure (script error or exceeded tick budget) returns the error and
  // discards any partially built overlay.
  std::expected<std::vector<pipeline::OverlayCommand>, Error> onFrame(
      const FrameView& frame
  );

 private:
  void bindApi();

  control::InputController& input_;
  std::chrono::milliseconds tickBudget_;
  std::optional<sol::state> lua_;
  std::string path_;
  sol::protected_function onFrame_;
  std::vector<pipeline::OverlayCommand> commands_;
  bool inTick_ = false;
  std::chrono::steady_clock::time_point tickStart_;

  static void budgetHook(lua_State* state, lua_Debug* debug);
};

}  // namespace ae::scripting
