#pragma once

#include "core/AssetStore.h"
#include "core/IStrategy.h"
#include "core/Renderer.h"
#include "core/Window.h"
#include "devices/Screen.h"
#include "ecs/Component.h"
#include "ecs/Entity.h"
#include "ecs/Registry.h"
#include "ecs/System.h"
#include "events/Bus.h"

#include "../components/TextLabelComponent.h"
#include "../layout/FPSWindow.h"
#include "../layout/ImageStreamWindow.h"
#include "../layout/ImageStreamWindowControlsWindow.h"
#include "../layout/LoadScriptWindow.h"
#include "../layout/LoggingWindow.h"
#include "../layout/MemoryWindow.h"
#include "../systems/GUISystem/GUISystem.h"
#include "../systems/RenderBoundingBoxSystem.h"
#include "../systems/RenderTextSystem.h"
#include "../systems/ScreenSystem.h"
#include "../systems/ScriptingSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10", "assets/fonts/Roboto-Medium.ttf", 24);
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10-small", "assets/fonts/Roboto-Medium.ttf", 16);

    //
    // TODO: remove temporary entity initialization

    ECS::Entity ball = ECS::Registry::Instance().CreateEntity();
    ECS::Registry::Instance().TagEntity(ball, "Ball");
    ECS::Registry::Instance().AddComponent<TextLabelComponent>(
        ball,
        cv::Vec2i(200, 200),
        "Hello world! [0, 0]"
    );
    ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
        ball,
        200,
        200,
        100,
        100
    );

    //
    // Initialize systems

    ECS::Registry::Instance().AddSystem<ScriptingSystem>(screen, scriptFile, lua);
    ECS::Registry::Instance().AddSystem<ScreenSystem>();
    ECS::Registry::Instance().AddSystem<GUISystem>();
    ECS::Registry::Instance().AddSystem<RenderTextSystem>();
    ECS::Registry::Instance().AddSystem<RenderBoundingBoxSystem>();

    //
    // Initialize windows

    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>(screen),
        GUISystemLayoutNodePosition::RIGHT_TOP
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<LoggingWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_LEFT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<MemoryWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<FPSWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindowControls>(screen),
        GUISystemLayoutNodePosition::LEFT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<LoadScriptWindow>(scriptFile, lua),
        GUISystemLayoutNodePosition::LEFT
    );
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance()
        .GetSystem<GUISystem>()
        .GetWindow<LoggingWindow>()
        .SubscribeToEvents();

    ECS::Registry::Instance().Update();

    if(screen.has_value()) {
      ECS::Registry::Instance().GetSystem<ScreenSystem>().Update(screen);
      ECS::Registry::Instance().GetSystem<ScriptingSystem>().Update();
    }
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    if(screen.has_value()) {
      ECS::Registry::Instance().GetSystem<RenderBoundingBoxSystem>().Render(screen);
    }

    ECS::Registry::Instance().GetSystem<GUISystem>().Render(
        renderer
    );
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance().GetSystem<GUISystem>().AfterRender();
  }

 private:
  std::optional<Devices::Screen> screen;

  // TODO: create state struct
  std::optional<std::string> scriptFile = "/Users/michaljarnot/IdeaProjects/swords-and-souls-scripts/dist/index.lua";
  std::optional<sol::state> lua = std::make_optional<sol::state>();
};
