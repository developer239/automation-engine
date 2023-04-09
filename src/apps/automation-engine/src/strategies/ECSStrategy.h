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
#include "../layout/LoggingWindow.h"
#include "../layout/MemoryWindow.h"
#include "../systems/GUISystem/GUISystem.h"
#include "../systems/RenderBoundingBoxSystem.h"
#include "../systems/RenderTextSystem.h"
#include "../systems/ScreenSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10", "assets/fonts/Roboto-Medium.ttf", 24);
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10-small", "assets/fonts/Roboto-Medium.ttf", 16);

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

    ECS::Registry::Instance().AddSystem<ScreenSystem>();
    ECS::Registry::Instance().AddSystem<GUISystem>();
    ECS::Registry::Instance().AddSystem<RenderTextSystem>();
    ECS::Registry::Instance().AddSystem<RenderBoundingBoxSystem>();

    //
    // Initialize windows

    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>(screen)
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<LoggingWindow>()
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<MemoryWindow>()
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<FPSWindow>()
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindowControls>(screen)
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
  std::optional<Devices::Screen> screen = Devices::Screen(800, 600, 0, 0);
};
