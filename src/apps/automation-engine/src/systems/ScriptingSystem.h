#pragma once

#include <sol/sol.hpp>

#include "devices/Screen.h"
#include "ecs/System.h"

class ScriptingSystem : public ECS::System {
 public:
  ScriptingSystem(
      std::optional<Devices::Screen>& screen, const std::string& scriptFile
  )
      : scriptFile(scriptFile) {
    lua.open_libraries(sol::lib::base);
    lua.script_file(scriptFile);

    int screenWidth = lua["screen"]["width"];
    int screenHeight = lua["screen"]["height"];
    int screenX = lua["screen"]["x"];
    int screenY = lua["screen"]["y"];

    screen = Devices::Screen(screenWidth, screenHeight, screenX, screenY);
  }

  void Update() { lua["onRender"](); }

 private:
  std::string scriptFile;
  sol::state lua;
};
