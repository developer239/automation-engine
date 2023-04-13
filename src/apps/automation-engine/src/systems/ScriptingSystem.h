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
    bool isOldScriptFile =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - scriptFileLoadedAt
        ).count() > 1;

    if (event.filePath != *scriptFile || isOldScriptFile) {
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

      LoadEntities();

      scriptFile = event.filePath;
      scriptFileLoadedAt = std::chrono::system_clock::now();
    }
  }

  void Update() { lua["main"]["onRender"](); }

 private:
  std::optional<Devices::Screen>& screen;

  std::chrono::time_point<std::chrono::system_clock> scriptFileLoadedAt;
  std::optional<std::string> scriptFile;
  sol::state lua;

  void LoadEntities() {
    ECS::Registry::Instance().RemoveAllEntitiesFromSystems();

    auto entities = lua["main"]["entities"];

    int i = 1;
    while (true) {
      sol::optional<sol::table> hasEntity = entities[i];
      if (hasEntity == sol::nullopt) {
        break;
      }

      sol::table entity = entities[i];
      ECS::Entity newEntity = ECS::Registry::Instance().CreateEntity();

      if (entity["tag"].valid()) {
        ECS::Registry::Instance().TagEntity(newEntity, entity["tag"]);
      }

      if (entity["group"].valid()) {
        ECS::Registry::Instance().GroupEntity(newEntity, entity["group"]);
      }

      // Components

      sol::optional<sol::table> hasComponents = entity["components"];
      if (hasComponents != sol::nullopt) {
        // Bounding Box

        sol::optional<sol::table> transform =
            entity["components"]["boundingBox"];
        if (transform != sol::nullopt) {
          ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
              newEntity,
              entity["components"]["boundingBox"]["position"]["x"],
              entity["components"]["boundingBox"]["position"]["y"],
              entity["components"]["boundingBox"]["size"]["width"],
              entity["components"]["boundingBox"]["size"]["height"]
          );
        }
      }

      i++;
    }
  }
};
