#include "scripting/ScriptRuntime.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>

#include <spdlog/spdlog.h>

namespace ae::scripting {

namespace {

// The budget hook needs the runtime that started the tick; scripts run on the
// control thread only, so one thread-local slot suffices.
thread_local const ScriptRuntime* tActiveRuntime = nullptr;
thread_local std::chrono::steady_clock::time_point tTickStart;
thread_local std::chrono::milliseconds tTickBudget{0};

// Instruction interval between budget checks - frequent enough to catch a
// runaway loop within a few milliseconds, rare enough to stay off profiles.
constexpr int kHookInstructionInterval = 200000;

// Top-level script execution at load time gets a looser budget than a frame
// tick: requiring modules and building tables legitimately takes longer.
constexpr std::chrono::milliseconds kLoadBudget{1000};

sol::variadic_results toLuaResult(
    sol::state_view lua, const control::InputResult& result
) {
  sol::variadic_results out;
  if (result.ok) {
    out.push_back({lua, sol::in_place, true});
    return out;
  }

  sol::table error = lua.create_table();
  error["code"] = result.code;
  error["message"] = result.message;
  out.push_back({lua, sol::in_place, sol::lua_nil});
  out.push_back({lua, sol::in_place, error});
  return out;
}

sol::variadic_results luaSuccess(sol::state_view lua) {
  sol::variadic_results out;
  out.push_back({lua, sol::in_place, true});
  return out;
}

sol::variadic_results luaRefusal(
    sol::state_view lua, const std::string_view code,
    const std::string_view message
) {
  sol::table error = lua.create_table();
  error["code"] = code;
  error["message"] = message;
  sol::variadic_results out;
  out.push_back({lua, sol::in_place, sol::lua_nil});
  out.push_back({lua, sol::in_place, error});
  return out;
}

std::optional<pipeline::OverlayColor> parseColor(const sol::object& object) {
  if (!object.valid() || object == sol::lua_nil) {
    return pipeline::OverlayColor{};  // defaults to opaque white
  }
  if (object.get_type() != sol::type::table) return std::nullopt;

  const sol::table table = object;
  const auto component = [&](const char* name,
                             const std::uint8_t fallback) -> std::optional<std::uint8_t> {
    const sol::object value = table[name];
    if (!value.valid() || value == sol::lua_nil) return fallback;
    if (value.get_type() != sol::type::number) return std::nullopt;
    const double number = value.as<double>();
    if (!std::isfinite(number) || number < 0 || number > 255) return std::nullopt;
    return static_cast<std::uint8_t>(number);
  };

  const auto r = component("r", 255);
  const auto g = component("g", 255);
  const auto b = component("b", 255);
  const auto a = component("a", 255);
  if (!r || !g || !b || !a) return std::nullopt;
  return pipeline::OverlayColor{.r = *r, .g = *g, .b = *b, .a = *a};
}

std::optional<double> finiteNumber(const sol::table& table, const char* name) {
  const sol::object value = table[name];
  if (!value.valid() || value.get_type() != sol::type::number) {
    return std::nullopt;
  }
  const double number = value.as<double>();
  if (!std::isfinite(number)) return std::nullopt;
  return number;
}

}  // namespace

void ScriptRuntime::budgetHook(lua_State* state, lua_Debug* /* debug */) {
  if (tActiveRuntime == nullptr) return;
  const auto elapsed = std::chrono::steady_clock::now() - tTickStart;
  if (elapsed > tTickBudget) {
    luaL_error(
        state, "script exceeded the %lld ms tick budget",
        static_cast<long long>(tTickBudget.count())
    );
  }
}

std::expected<void, Error> ScriptRuntime::load(const std::string& path) {
  if (!std::filesystem::exists(path)) {
    return std::unexpected(Error::scriptLoadFailed(path, "file does not exist"));
  }

  unload();
  lua_.emplace();
  lua_->open_libraries(
      sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table,
      sol::lib::math, sol::lib::os
  );
  // os.exit would skip C++ teardown entirely (held keys stay down); a blocked
  // os.execute would freeze the control thread outside the budget hook's
  // reach. Scripts get neither.
  (*lua_)["os"]["exit"] = sol::lua_nil;
  (*lua_)["os"]["execute"] = sol::lua_nil;
  bindApi();

  // The budget hook guards script loading too - a top-level infinite loop
  // must not hang the control thread forever.
  lua_sethook(
      lua_->lua_state(), &ScriptRuntime::budgetHook, LUA_MASKCOUNT,
      kHookInstructionInterval
  );
  tActiveRuntime = this;
  tTickStart = std::chrono::steady_clock::now();
  tTickBudget = kLoadBudget;
  const auto result = lua_->safe_script_file(path, sol::script_pass_on_error);
  tActiveRuntime = nullptr;

  if (!result.valid()) {
    const sol::error error = result;
    unload();
    return std::unexpected(Error::scriptLoadFailed(path, error.what()));
  }

  const sol::object main = (*lua_)["main"];
  if (main.get_type() != sol::type::table) {
    unload();
    return std::unexpected(
        Error::scriptLoadFailed(path, "script must define a 'main' table")
    );
  }
  const sol::object onFrame = sol::table(main)["onFrame"];
  if (onFrame.get_type() != sol::type::function) {
    unload();
    return std::unexpected(Error::scriptLoadFailed(
        path, "'main.onFrame' must be a function"
    ));
  }

  onFrame_ = sol::protected_function(onFrame);
  path_ = path;

  spdlog::info("[Script] loaded '{}'", path);
  return {};
}

void ScriptRuntime::unload() {
  onFrame_ = sol::protected_function{};
  lua_.reset();
  path_.clear();
  commands_.clear();
  inTick_ = false;
}

std::expected<std::vector<pipeline::OverlayCommand>, Error>
ScriptRuntime::onFrame(const FrameView& frame) {
  commands_.clear();
  inTick_ = true;
  tickStart_ = std::chrono::steady_clock::now();
  tActiveRuntime = this;
  tTickStart = tickStart_;
  tTickBudget = tickBudget_;

  sol::table frameTable = lua_->create_table();
  frameTable["seq"] = frame.seq;
  frameTable["width"] = frame.width;
  frameTable["height"] = frame.height;
  frameTable["ageMs"] = frame.ageMs;
  frameTable["deltaMs"] = frame.deltaMs;
  frameTable["isFocused"] = frame.focused;

  const auto result = onFrame_(frameTable);

  inTick_ = false;
  tActiveRuntime = nullptr;

  if (!result.valid()) {
    const sol::error error = result;
    commands_.clear();
    return std::unexpected(Error::scriptRuntimeError(error.what()));
  }
  return std::exchange(commands_, {});
}

void ScriptRuntime::bindApi() {
  sol::state& lua = *lua_;

  sol::table logTable = lua.create_named_table("log");
  logTable["info"] = [](const std::string& message) { spdlog::info("[Lua] {}", message); };
  logTable["warn"] = [](const std::string& message) { spdlog::warn("[Lua] {}", message); };
  logTable["error"] = [](const std::string& message) { spdlog::error("[Lua] {}", message); };

  // Overlay commands accumulate into the tick's buffer; the buffer is
  // published by the control thread only when onFrame completes successfully.
  sol::table overlay = lua.create_named_table("overlay");
  overlay["box"] = [this, &lua](const sol::table& spec) {
    if (!inTick_) {
      return luaRefusal(lua, "outside_tick", "overlay is only available inside onFrame");
    }
    const auto x = finiteNumber(spec, "x");
    const auto y = finiteNumber(spec, "y");
    const auto width = finiteNumber(spec, "w");
    const auto height = finiteNumber(spec, "h");
    const auto color = parseColor(spec["color"]);
    if (!x || !y || !width || !height || *width <= 0 || *height <= 0 || !color) {
      return luaRefusal(
          lua, "invalid_overlay",
          "box needs finite x, y, positive w, h, and color components in 0-255"
      );
    }
    commands_.push_back(pipeline::OverlayBox{
        .x = static_cast<float>(*x),
        .y = static_cast<float>(*y),
        .width = static_cast<float>(*width),
        .height = static_cast<float>(*height),
        .color = *color,
        .thickness = static_cast<float>(spec.get_or("thickness", 2.0)),
        .filled = spec.get_or("filled", false),
    });
    return luaSuccess(lua);
  };
  overlay["line"] = [this, &lua](const sol::table& spec) {
    if (!inTick_) {
      return luaRefusal(lua, "outside_tick", "overlay is only available inside onFrame");
    }
    const auto x1 = finiteNumber(spec, "x1");
    const auto y1 = finiteNumber(spec, "y1");
    const auto x2 = finiteNumber(spec, "x2");
    const auto y2 = finiteNumber(spec, "y2");
    const auto color = parseColor(spec["color"]);
    if (!x1 || !y1 || !x2 || !y2 || !color) {
      return luaRefusal(
          lua, "invalid_overlay",
          "line needs finite x1, y1, x2, y2, and color components in 0-255"
      );
    }
    commands_.push_back(pipeline::OverlayLine{
        .x1 = static_cast<float>(*x1),
        .y1 = static_cast<float>(*y1),
        .x2 = static_cast<float>(*x2),
        .y2 = static_cast<float>(*y2),
        .color = *color,
        .thickness = static_cast<float>(spec.get_or("thickness", 2.0)),
    });
    return luaSuccess(lua);
  };
  overlay["text"] = [this, &lua](const sol::table& spec) {
    if (!inTick_) {
      return luaRefusal(lua, "outside_tick", "overlay is only available inside onFrame");
    }
    const auto x = finiteNumber(spec, "x");
    const auto y = finiteNumber(spec, "y");
    const sol::object text = spec["text"];
    const auto color = parseColor(spec["color"]);
    if (!x || !y || text.get_type() != sol::type::string || !color) {
      return luaRefusal(
          lua, "invalid_overlay",
          "text needs finite x, y, a text string, and color components in 0-255"
      );
    }
    commands_.push_back(pipeline::OverlayText{
        .x = static_cast<float>(*x),
        .y = static_cast<float>(*y),
        .text = text.as<std::string>(),
        .color = *color,
    });
    return luaSuccess(lua);
  };

  sol::table input = lua.create_named_table("input");
  input["move"] = [this, &lua](const double x, const double y) {
    return toLuaResult(lua, input_.moveMouse(x, y, /* smooth = */ false));
  };
  input["moveSmooth"] = [this, &lua](const double x, const double y) {
    return toLuaResult(lua, input_.moveMouse(x, y, /* smooth = */ true));
  };
  input["click"] = [this, &lua](
                       const double x, const double y,
                       const sol::optional<std::string> button
                   ) {
    return toLuaResult(lua, input_.click(x, y, button.value_or("left")));
  };
  input["pressButton"] = [this, &lua](const std::string& button) {
    return toLuaResult(lua, input_.pressButton(button));
  };
  input["releaseButton"] = [this, &lua](const std::string& button) {
    return toLuaResult(lua, input_.releaseButton(button));
  };
  input["tap"] = [this, &lua](const std::string& key) {
    return toLuaResult(lua, input_.tapKey(key));
  };
  input["press"] = [this, &lua](const std::string& key) {
    return toLuaResult(lua, input_.pressKey(key));
  };
  input["release"] = [this, &lua](const std::string& key) {
    return toLuaResult(lua, input_.releaseKey(key));
  };
  input["typeText"] = [this, &lua](const std::string& text) {
    return toLuaResult(lua, input_.typeText(text));
  };

  sol::table target = lua.create_named_table("target");
  target["focus"] = [this, &lua] {
    return toLuaResult(lua, input_.focusTarget());
  };

  // Validated key constants (keys.W == "w") so typos fail at script load
  // rather than as unknown_key refusals at runtime.
  sol::table keys = lua.create_named_table("keys");
  for (const auto& [name, key] : control::keyNameTable()) {
    std::string constant{name};
    std::ranges::transform(constant, constant.begin(), [](const unsigned char c) {
      return static_cast<char>(std::toupper(c));
    });
    keys[constant] = std::string{name};
  }
}

}  // namespace ae::scripting
