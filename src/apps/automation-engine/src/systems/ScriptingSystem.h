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
    BindVector<ECS::Entity>();
    BindPrintTable();
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

      const sol::optional<sol::table> groupsTable = entity["groups"];
      const sol::table defaultTable;
      const std::vector<std::string> groups =
          groupsTable ? groupsTable.value().as<std::vector<std::string>>()
                      : std::vector<std::string>();
      for (const auto& group : groups) {
        ECS::Registry::Instance().GroupEntity(newEntity, group);
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
        "createEntity",
        &ECS::Registry::CreateEntity,
        "tagEntity",
        &ECS::Registry::TagEntity,
        "getEntityByTag",
        &ECS::Registry::GetEntityByTag,
        "groupEntity",
        &ECS::Registry::GroupEntity,
        "getEntityGroups",
        [this](ECS::Registry& registry, ECS::Entity entity) {
          std::vector<std::string> groups = registry.GetEntityGroups(entity);
          return vectorToTable<std::string>(groups);
        },
        "getEntitiesByGroup",
        &ECS::Registry::GetEntitiesByGroup,
        "removeEntityGroup",
        &ECS::Registry::RemoveEntityGroup,
        "removeEntityGroups",
        &ECS::Registry::RemoveEntityGroups
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

  void BindPrintTable() {
    lua.set_function("printTable", [this](sol::table table) {
      PrintTable(table);
    });
  }

  template <typename TType>
  void BindVector() {
    using VectorType = std::vector<TType>;

    // Bind VectorType to Lua using sol::new_usertype
    lua.new_usertype<VectorType>(
        sol::meta_function::construct,
        sol::factories([]() { return VectorType{}; }),
        "size",
        &VectorType::size,
        "at",
        [](VectorType& v, std::size_t i) -> TType& {
          if (i >= v.size()) {
            throw sol::error("index out of bounds");
          }
          return v[i];
        },
        sol::meta_function::to_string,
        [](const VectorType& v) {
          std::ostringstream oss;

          oss << "Vector [";
          for (std::size_t i = 0; i < v.size(); ++i) {
            if constexpr (std::is_same_v<TType, ECS::Entity>) {
              oss << v[i].ToString();
            } else {
              oss << "unknown";
            }

            if (i < v.size() - 1) {
              oss << ", ";
            }
          }
          oss << "]";

          return oss.str();
        }
    );
  }

  // TODO: possibly implement specialisation and convert to sol::table before printing
  // NOTE: "std::vector<TType>" is printed as "Table []"
  void PrintTable(const sol::table table) {
    std::cout << "Table [";

    auto size = table.size();
    int count = 0;
    for (auto& pair : table) {
      if (pair.first.is<int>()) {
        std::cout << pair.first.as<int>() << ": ";
      } else if (pair.first.is<std::string>()) {
        std::cout << pair.first.as<std::string>() << ": ";
      }

      if (pair.second.is<sol::table>()) {
        PrintTable(pair.second.as<sol::table>());
      } else if (pair.second.is<std::string>()) {
        std::cout << pair.second.as<std::string>();
      } else if (pair.second.is<int>()) {
        std::cout << pair.second.as<int>();
      } else {
        std::cout << "unknown";
      }

      if (count < size - 1) {
        std::cout << ", ";
      }
      count++;
    }

    std::cout << "]";
  }

  template <typename TType>
  sol::table vectorToTable(const std::vector<TType>& vec) {
    sol::table result = lua.create_table();
    int index = 1;

    for (const auto& str : vec) {
      result[index++] = str;
    }

    return result;
  }
};
