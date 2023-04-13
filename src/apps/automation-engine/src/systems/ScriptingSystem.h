#pragma once

#include <sol/sol.hpp>

#include "devices/Screen.h"
#include "ecs/System.h"

class ScriptingSystem : public ECS::System {
 public:
  ScriptingSystem(
      std::optional<Devices::Screen>& screen,
      std::optional<std::string>& scriptFile, std::optional<sol::state>& lua
  )
      : screen(screen), scriptFile(scriptFile), lua(lua) {
    LoadFile();
  }

  void LoadFile() {
    if (scriptFile.has_value()) {
      lua->open_libraries(sol::lib::base);
      lua->script_file(*scriptFile);

      int screenWidth = (*lua)["screen"]["width"];
      int screenHeight = (*lua)["screen"]["height"];
      int screenX = (*lua)["screen"]["x"];
      int screenY = (*lua)["screen"]["y"];

      screen = Devices::Screen(screenWidth, screenHeight, screenX, screenY);
    }
  }

  void Update() {
    if (lua.has_value()) {
      (*lua)["onRender"]();
    }
  }

 private:
  std::optional<Devices::Screen>& screen;
  std::optional<std::string>& scriptFile;
  std::optional<sol::state>& lua;
};
