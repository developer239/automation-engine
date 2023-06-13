#pragma once

#include <ctime>
#include <iomanip>
#include <memory>
#include <sol/sol.hpp>
#include <sstream>
#include <vector>

#include "devices/Keyboard.h"
#include "devices/Mouse.h"
#include "devices/Screen.h"
#include "ecs/System.h"
#include "events/Bus.h"

#include "../components/DetectObjectsComponent.h"
#include "../components/DetectTextComponent.h"
#include "../components/EditableComponent.h"
#include "../services/GlobalKeyboardListener.h"

class ScriptingSystem : public ECS::System {
 public:
  ScriptingSystem(std::optional<Devices::Screen>& screen, bool& isRunning)
      : screen(screen), lua(), isRunning(isRunning) {
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
    BindEditableComponent();
    BindTextLabelComponent();
    BindBoundingBoxComponent();
    BindAppColorStruct();
    BindAppPositionStruct();
    BindAppSizeStruct();
    BindMouse();
    BindKeyboard();
    BindUtils();
    BindEventBus();
  }

  void SubscribeToEvents() {
    Events::Bus::Instance().SubscribeToEvent<ScriptFileSelectedEvent>(
        this,
        &ScriptingSystem::OnFileSelected
    );
    Events::Bus::Instance().SubscribeToEvent<KeyPressedEvent>(
        this,
        &ScriptingSystem::OnKeyPressedEvent
    );
  }

  void OnKeyPressedEvent(KeyPressedEvent& event) {
    if (event.asciiSymbol == "p") {
      isRunning = !isRunning;
    }
  }

  void OnFileSelected(ScriptFileSelectedEvent& event) {
    lua.script_file(event.filePath);

    ECS::Registry::Instance().RemoveAllEntitiesFromSystems();

    LoadScreenInfo();

    scriptFile = event.filePath;
  }

  void Update() {
    if (isRunning) {
      try {
        lua["main"]["onUpdate"]();
      } catch (const sol::error& error) {
        std::cout << "Error: " << error.what() << std::endl;
      } catch (const std::exception& error) {
        std::cout << "Error: " << error.what() << std::endl;
      }
    }
  }

 private:
  bool& isRunning;
  GlobalKeyboardListener globalKeyboardListener = GlobalKeyboardListener();

  std::optional<Devices::Screen>& screen;

  std::optional<std::string> scriptFile;
  sol::state lua;

  void LoadScreenInfo() {
    int screenWidth = lua["main"]["screen"]["width"];
    int screenHeight = lua["main"]["screen"]["height"];
    int screenX = lua["main"]["screen"]["x"];
    int screenY = lua["main"]["screen"]["y"];

    screen = Devices::Screen(screenWidth, screenHeight, screenX, screenY);
  }

  void BindUtils() {
    lua.set_function(
        "checkCollision",
        [](ECS::Entity& entityA, ECS::Entity& entityB) {
          auto& boundingBoxA =
              ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(
                  entityA
              );
          auto& boundingBoxB =
              ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(
                  entityB
              );

          bool xOverlap = boundingBoxA.position.x + boundingBoxA.size.width >=
                              boundingBoxB.position.x &&
                          boundingBoxB.position.x + boundingBoxB.size.width >=
                              boundingBoxA.position.x;
          bool yOverlap = boundingBoxA.position.y + boundingBoxA.size.height >=
                              boundingBoxB.position.y &&
                          boundingBoxB.position.y + boundingBoxB.size.height >=
                              boundingBoxA.position.y;

          bool touching =
              xOverlap && yOverlap &&
              boundingBoxA.position.x <
                  boundingBoxB.position.x + boundingBoxB.size.width &&
              boundingBoxB.position.x <
                  boundingBoxA.position.x + boundingBoxA.size.width &&
              boundingBoxA.position.y <
                  boundingBoxB.position.y + boundingBoxB.size.height &&
              boundingBoxB.position.y <
                  boundingBoxA.position.y + boundingBoxA.size.height;

          return touching;
        }
    );

    lua.set_function("getTicks", []() { return SDL_GetTicks(); });

    lua.set_function(
        "sortByX",
        [](std::vector<ECS::Entity>& entities, bool descending = false) {
          std::sort(
              entities.begin(),
              entities.end(),
              [descending](const ECS::Entity& a, const ECS::Entity& b) {
                if (!descending) {
                  return ECS::Registry::Instance()
                             .GetComponent<BoundingBoxComponent>(a)
                             .position.x <
                         ECS::Registry::Instance()
                             .GetComponent<BoundingBoxComponent>(b)
                             .position.x;
                }

                return ECS::Registry::Instance()
                           .GetComponent<BoundingBoxComponent>(a)
                           .position.x >
                       ECS::Registry::Instance()
                           .GetComponent<BoundingBoxComponent>(b)
                           .position.x;
              }
          );
        }
    );
  }

  void BindRegistry() {
    lua.new_usertype<ECS::Registry>(
        "Registry",
        "Instance",
        &ECS::Registry::Instance,
        "createEntity",
        &ECS::Registry::CreateEntity,
        "killEntity",
        &ECS::Registry::KillEntity,
        "getEntityByTag",
        &ECS::Registry::GetEntityByTag,
        "getEntitiesByGroup",
        &ECS::Registry::GetEntitiesByGroup
    );
  }

  void BindEventBus() {
    lua.new_usertype<Events::Bus>(
        "Bus",
        "Instance",
        &Events::Bus::Instance,
        "emitMessageEvent",
        [](Events::Bus& bus, const std::string& message) {
          std::time_t currentTime = std::time(nullptr);
          std::tm localTime;
          localtime_r(&currentTime, &localTime);

          std::ostringstream timeStream;
          timeStream << "[" << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
                     << "] " << message;

          std::string formattedMessage = timeStream.str();

          bus.EmitEvent<MessageEvent>(formattedMessage);
        }
    );
  }

  void BindEntity() {
    lua.new_usertype<ECS::Entity>(
        "Entity",
        sol::constructors<ECS::Entity(int)>(),
        "getId",
        &ECS::Entity::GetId,
        sol::meta_function::to_string,
        &ECS::Entity::ToString,
        "setTag",
        [](ECS::Entity entity, const std::string& tag) {
          ECS::Registry::Instance().TagEntity(entity, tag);
        },
        "getTag",
        [](ECS::Entity entity) {
          return ECS::Registry::Instance().GetEntityTag(entity);
        },
        "addGroup",
        [](ECS::Entity entity, const std::string& group) {
          ECS::Registry::Instance().GroupEntity(entity, group);
        },
        "getGroups",
        [this](ECS::Entity entity) {
          std::vector<std::string> groups =
              ECS::Registry::Instance().GetEntityGroups(entity);
          return VectorToTable<std::string>(groups);
        },
        "removeGroup",
        [](ECS::Entity entity, const std::string& group) {
          ECS::Registry::Instance().RemoveEntityGroup(entity, group);
        },
        "removeGroups",
        [](ECS::Entity entity) {
          ECS::Registry::Instance().RemoveEntityGroups(entity);
        },
        "getComponentBoundingBox",
        [this](ECS::Entity entity) {
          if (!ECS::Registry::Instance().HasComponent<BoundingBoxComponent>(
                  entity
              )) {
            throw std::runtime_error("Entity does not have BoundingBoxComponent"
            );
          }

          return sol::object(
              lua,
              sol::in_place,
              ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(
                  entity
              )
          );
        },
        "addComponentBoundingBox",
        [](ECS::Entity entity, const sol::table& bboxTable) {
          auto bbox = BoundingBoxComponent(
              App::Position(
                  bboxTable["position"]["x"],
                  bboxTable["position"]["y"]
              ),
              App::Size(
                  bboxTable["size"]["width"],
                  bboxTable["size"]["height"]
              ),
              App::Color(
                  bboxTable["color"]["r"],
                  bboxTable["color"]["g"],
                  bboxTable["color"]["b"]
              ),
              bboxTable["thickness"]
          );

          ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
              entity,
              bbox
          );
        },
        "getComponentEditable",
        [this](ECS::Entity entity) {
          if (!ECS::Registry::Instance().HasComponent<EditableComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have EditableComponent");
          }

          return sol::object(
              lua,
              sol::in_place,
              ECS::Registry::Instance().GetComponent<EditableComponent>(entity)
          );
        },
        "addComponentEditable",
        [](ECS::Entity entity) {
          ECS::Registry::Instance().AddComponent<EditableComponent>(entity);
        },
        "getComponentTextLabel",
        [this](ECS::Entity entity) {
          if (!ECS::Registry::Instance().HasComponent<TextLabelComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have TextLabelComponent");
          }

          return sol::object(
              lua,
              sol::in_place,
              ECS::Registry::Instance().GetComponent<TextLabelComponent>(entity)
          );
        },
        "addComponentTextLabel",
        [](ECS::Entity entity, const sol::table& bboxTable) {
          auto textLabel = TextLabelComponent(
              bboxTable["text"],
              App::Position(
                  bboxTable["position"]["x"],
                  bboxTable["position"]["y"]
              ),
              App::Color(
                  bboxTable["color"]["r"],
                  bboxTable["color"]["g"],
                  bboxTable["color"]["b"]
              )
          );

          ECS::Registry::Instance().AddComponent<TextLabelComponent>(
              entity,
              textLabel
          );
        },
        "addComponentDetection",
        [](ECS::Entity entity) {
          ECS::Registry::Instance().AddComponent<DetectionComponent>(entity);
        },
        "addComponentDetectionMorphologyOperation",
        [](ECS::Entity entity, std::string operationType, const sol::table& data
        ) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          auto& detectionComponent =
              ECS::Registry::Instance().GetComponent<DetectionComponent>(entity
              );

          if (operationType == "close") {
            detectionComponent.AddArguments(std::make_shared<CloseOperation>(
                App::Size(data["size"]["width"], data["size"]["height"])
            ));
          }

          if (operationType == "dilate") {
            detectionComponent.AddArguments(std::make_shared<DilateOperation>(
                App::Size(data["size"]["width"], data["size"]["height"])
            ));
          }

          if (operationType == "erode") {
            detectionComponent.AddArguments(std::make_shared<ErodeOperation>(
                App::Size(data["size"]["width"], data["size"]["height"])
            ));
          }

          if (operationType == "open") {
            detectionComponent.AddArguments(std::make_shared<OpenOperation>(
                App::Size(data["size"]["width"], data["size"]["height"])
            ));
          }
        },
        "addComponentDetectionColorsOperation",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          auto& detectionComponent =
              ECS::Registry::Instance().GetComponent<DetectionComponent>(entity
              );

          detectionComponent.AddArguments(
              std::make_shared<DetectColorsOperation>(
                  App::Color(
                      data["lowerBound"]["r"],
                      data["lowerBound"]["g"],
                      data["lowerBound"]["b"]
                  ),
                  App::Color(
                      data["upperBound"]["r"],
                      data["upperBound"]["g"],
                      data["upperBound"]["b"]
                  )
              )
          );
        },
        "addComponentDetectionCropOperation",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          auto& detectionComponent =
              ECS::Registry::Instance().GetComponent<DetectionComponent>(entity
              );

          detectionComponent.AddArguments(std::make_shared<CropOperation>(
              App::Position(data["position"]["x"], data["position"]["y"]),
              App::Size(data["size"]["width"], data["size"]["height"])
          ));
        },
        "addComponentDetectContours",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          auto detectContoursComponent = DetectContoursComponent(
              data["id"],
              App::Size(data["minArea"]["width"], data["minArea"]["height"]),
              App::Color(
                  data["bboxColor"]["r"],
                  data["bboxColor"]["g"],
                  data["bboxColor"]["b"]
              )
          );

          if (data["shouldRenderPreview"].valid()) {
            detectContoursComponent.shouldRenderPreview =
                data["shouldRenderPreview"];
          }

          if (data["bboxThickness"].valid()) {
            detectContoursComponent.bboxThickness = data["bboxThickness"];
          }

          if (data["maxArea"].valid()) {
            detectContoursComponent.maxArea =
                App::Size(data["maxArea"]["width"], data["maxArea"]["height"]);
          }

          ECS::Registry::Instance().AddComponent<DetectContoursComponent>(
              entity,
              detectContoursComponent
          );
        },
        "addComponentDetectText",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          auto detectTextComponent = DetectTextComponent(
              data["id"],
              App::Color(
                  data["bboxColor"]["r"],
                  data["bboxColor"]["g"],
                  data["bboxColor"]["b"]
              )
          );

          if (data["shouldRenderPreview"].valid()) {
            detectTextComponent.shouldRenderPreview =
                data["shouldRenderPreview"];
          }

          if (data["bboxThickness"].valid()) {
            detectTextComponent.bboxThickness = data["bboxThickness"];
          }

          ECS::Registry::Instance().AddComponent<DetectTextComponent>(
              entity,
              detectTextComponent
          );
        },
        "addComponentDetectObjects",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          ECS::Registry::Instance().AddComponent<DetectObjectsComponent>(
              entity,
              data["id"],
              data["confidenceThreshold"],
              data["nonMaximumSuppressionThreshold"],
              data["pathToModel"],
              data["pathToClasses"]
          );
        },
        "addComponentInstanceSegmentation",
        [](ECS::Entity entity, const sol::table& data) {
          if (!ECS::Registry::Instance().HasComponent<DetectionComponent>(entity
              )) {
            throw std::runtime_error("Entity does not have DetectionComponent");
          }

          ECS::Registry::Instance().AddComponent<InstanceSegmentationComponent>(
              entity,
              data["id"],
              data["confidenceThreshold"],
              data["nonMaximumSuppressionThreshold"],
              data["pathToModel"],
              data["pathToClasses"]
          );
        },
        "addComponentOdometer",
        [](ECS::Entity entity, const sol::table& data) {
          ECS::Registry::Instance().AddComponent<OdometerComponent>(
              entity,
              data["isRunning"],
              data["shouldDrawArrow"],
              data["shouldDebugMatches"]
          );
        }
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

  void BindEditableComponent() {
    lua.new_usertype<EditableComponent>(
        "EditableComponent",
        sol::constructors<EditableComponent(), EditableComponent(const bool&)>(
        ),
        "isEditable",
        &EditableComponent::isEditable
    );
  }

  void BindTextLabelComponent() {
    lua.new_usertype<TextLabelComponent>(
        "TextLabelComponent",
        sol::constructors<
            TextLabelComponent(),
            TextLabelComponent(const std::string&, const App::Position&, const App::Color&, const std::string&)>(
        ),
        "position",
        &TextLabelComponent::position,
        "text",
        &TextLabelComponent::text,
        "color",
        &TextLabelComponent::color,
        "fontId",
        &TextLabelComponent::fontId
    );
  }

  void BindBoundingBoxComponent() {
    lua.new_usertype<BoundingBoxComponent>(
        "BoundingBoxComponent",
        sol::constructors<
            BoundingBoxComponent(),
            BoundingBoxComponent(const App::Position&, const App::Size&, const App::Color&)>(
        ),
        "position",
        &BoundingBoxComponent::position,
        "size",
        &BoundingBoxComponent::size,
        "color",
        &BoundingBoxComponent::color,
        "thickness",
        &BoundingBoxComponent::thickness
    );
  }

  void BindAppPositionStruct() {
    lua.new_usertype<App::Position>(
        "Position",
        sol::constructors<App::Position(int, int)>(),
        "x",
        &App::Position::x,
        "y",
        &App::Position::y
    );
  }

  void BindAppColorStruct() {
    lua.new_usertype<App::Color>(
        "Color",
        sol::constructors<App::Color(int, int, int)>(),
        "r",
        &App::Color::r,
        "g",
        &App::Color::g,
        "b",
        &App::Color::b
    );
  }

  void BindAppSizeStruct() {
    lua.new_usertype<App::Size>(
        "Size",
        sol::constructors<App::Size(int, int)>(),
        "width",
        &App::Size::width,
        "height",
        &App::Size::height
    );
  }

  void BindMouse() {
    lua.new_usertype<Devices::Mouse>(
        "Mouse",
        "Instance",
        &Devices::Mouse::Instance,
        "move",
        &Devices::Mouse::Move
    );
  }

  void BindKeyboard() {
    lua.new_usertype<Devices::Keyboard>(
        "Keyboard",
        "Instance",
        &Devices::Keyboard::Instance,
        "type",
        &Devices::Keyboard::Type,
        "enter",
        &Devices::Keyboard::Enter,
        "arrowUp",
        &Devices::Keyboard::ArrowUp,
        "arrowDown",
        &Devices::Keyboard::ArrowDown,
        "arrowLeft",
        &Devices::Keyboard::ArrowLeft,
        "arrowRight",
        &Devices::Keyboard::ArrowRight,
        "escape",
        &Devices::Keyboard::Escape,
        "space",
        &Devices::Keyboard::Space
    );
  }

  // TODO: possibly implement specialisation and convert to sol::table before
  // printing NOTE: "std::vector<TType>" is printed as "Table []"
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
  sol::table VectorToTable(const std::vector<TType>& vec) {
    sol::table result = lua.create_table();
    int index = 1;

    for (const auto& str : vec) {
      result[index++] = str;
    }

    return result;
  }
};
