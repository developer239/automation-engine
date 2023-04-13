#pragma once

#include <sol/sol.hpp>

#include "devices/Screen.h"
#include "ecs/System.h"
#include "events/Bus.h"

class ScriptingSystem : public ECS::System {
 public:
  ScriptingSystem(std::optional<Devices::Screen>& screen)
      : screen(screen), lua() {}

  void SubscribeToEvents() {
    Events::Bus::Instance().SubscribeToEvent<ScriptFileSelectedEvent>(
        this,
        &ScriptingSystem::OnFileSelected
    );
  }

  void OnFileSelected(ScriptFileSelectedEvent& event) {
    if (event.filePath != *scriptFile) {
      lua.open_libraries(
          sol::lib::base,
          sol::lib::package,
          sol::lib::string,
          sol::lib::table,
          sol::lib::math,
          sol::lib::os
      );
      lua.script_file(event.filePath);

      int screenWidth = lua["main"]["screen"]["width"];
      int screenHeight = lua["main"]["screen"]["height"];
      int screenX = lua["main"]["screen"]["x"];
      int screenY = lua["main"]["screen"]["y"];

      screen = Devices::Screen(screenWidth, screenHeight, screenX, screenY);

      scriptFile = event.filePath;
    }
  }

  void Update() { lua["main"]["onRender"](); }

 private:
  std::optional<Devices::Screen>& screen;
  std::optional<std::string> scriptFile;
  sol::state lua;
};
