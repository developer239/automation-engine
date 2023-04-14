#pragma once

#include <sol/sol.hpp>
#include <vector>

#include "devices/Screen.h"
#include "ecs/System.h"
#include "events/Bus.h"

#include "../components/EditableComponent.h"

class ScriptingSystem : public ECS::System {
 public:
  ScriptingSystem(std::optional<Devices::Screen>& screen)
      : screen(screen), lua() {
    lua.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table,
        sol::lib::math,
        sol::lib::os
    );

    BindRegistry();
    BindEntity();
    BindVector();
  }

  void SubscribeToEvents() {
    Events::Bus::Instance().SubscribeToEvent<ScriptFileSelectedEvent>(
        this,
        &ScriptingSystem::OnFileSelected
    );
  }

  void OnFileSelected(ScriptFileSelectedEvent& event) {
    // NOTE: it seems that ScriptFileSelectedEvent is be emitted multiple times
    bool isOldScriptFile =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - scriptFileLoadedAt
        )
            .count() > 1;

    if (event.filePath != *scriptFile || isOldScriptFile) {
      lua.script_file(event.filePath);

      LoadScreenInfo();
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

  void LoadScreenInfo() {
    int screenWidth = lua["main"]["screen"]["width"];
    int screenHeight = lua["main"]["screen"]["height"];
    int screenX = lua["main"]["screen"]["x"];
    int screenY = lua["main"]["screen"]["y"];

    screen = Devices::Screen(screenWidth, screenHeight, screenX, screenY);
  }

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
        // Editable

        // is entity["isEditable"] true
        if (entity["isEditable"].valid() && entity["isEditable"]) {
          ECS::Registry::Instance().AddComponent<EditableComponent>(newEntity);
        }

        // BoundingBox
        sol::optional<sol::table> bbox = entity["components"]["boundingBox"];
        if (bbox != sol::nullopt) {
          ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
              newEntity,
              entity["components"]["boundingBox"]["position"]["x"],
              entity["components"]["boundingBox"]["position"]["y"],
              entity["components"]["boundingBox"]["size"]["width"],
              entity["components"]["boundingBox"]["size"]["height"]
          );

          sol::optional<sol::table> color =
              entity["components"]["boundingBox"]["color"];
          if (color != sol::nullopt) {
            ECS::Registry::Instance()
                .GetComponent<BoundingBoxComponent>(newEntity)
                .color = cv::Scalar(
                entity["components"]["boundingBox"]["color"]["b"],
                entity["components"]["boundingBox"]["color"]["g"],
                entity["components"]["boundingBox"]["color"]["r"]
            );
          }
        }

        // TextLabel
        sol::optional<sol::table> textLabel = entity["components"]["textLabel"];
        if (textLabel != sol::nullopt) {
          ECS::Registry::Instance().AddComponent<TextLabelComponent>(
              newEntity,
              cv::Vec2i(
                  entity["components"]["textLabel"]["position"]["x"],
                  entity["components"]["textLabel"]["position"]["y"]
              ),
              entity["components"]["textLabel"]["text"],
              SDL_Color(
                  {entity["components"]["textLabel"]["color"]["r"],
                   entity["components"]["textLabel"]["color"]["g"],
                   entity["components"]["textLabel"]["color"]["b"]}
              )
          );
        }
      }

      i++;
    }
  }

  void BindRegistry() {
    lua.new_usertype<ECS::Registry>(
        "Registry",
        "Instance",
        &ECS::Registry::Instance,
        "getEntityByTag",
        &ECS::Registry::GetEntityByTag,
        "getEntitiesByGroup",
        &ECS::Registry::GetEntitiesByGroup,
        "createEntity",
        &ECS::Registry::CreateEntity,
        "tagEntity",
        &ECS::Registry::TagEntity,
        "groupEntity",
        &ECS::Registry::GroupEntity
    );
  }

  void BindEntity() {
    lua.new_usertype<ECS::Entity>(
        "Entity",
        sol::constructors<ECS::Entity(int)>(),
        "getId",
        &ECS::Entity::GetId,
        sol::meta_function::to_string,
        &ECS::Entity::ToString
    );
  }

  void BindVector() {
    using EntityVector = std::vector<ECS::Entity>;

    // Bind EntityVector to Lua using sol::new_usertype
    lua.new_usertype<EntityVector>(
        "EntityVector",
        sol::constructors<EntityVector()>(),
        "size",
        &EntityVector::size,
        "at",
        [](EntityVector& v, std::size_t i) -> ECS::Entity& {
          if (i >= v.size()) {
            throw sol::error("index out of bounds");
          }
          return v[i];
        },
        sol::meta_function::to_string,
        [](const EntityVector& v) {
          std::string s = "EntityVector {";
          for (std::size_t i = 0; i < v.size(); ++i) {
            s += v[i].ToString();

            if (i < v.size() - 1) {
              s += ", ";
            }
          }
          s += "}";
          return s;
        }
    );
  }
};
