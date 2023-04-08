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
#include "../systems/RenderTextSystem.h"
#include "../systems/ScreenSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    Core::AssetStore::Instance().AddFont(
        "pico8-font-10",
        "assets/fonts/Roboto-Medium.ttf",
        24
    );
    Core::AssetStore::Instance().AddFont(
        "pico8-font-10-small",
        "assets/fonts/Roboto-Medium.ttf",
        16
    );

    ECS::Entity ball = ECS::Registry::Instance().CreateEntity();
    ECS::Registry::Instance().TagEntity(ball, "Ball");
    ECS::Registry::Instance().AddComponent<TextLabelComponent>(ball, cv::Vec2i(0, 200), "asdf asdf asd fasf asdf asdfasdf asfda");

    //
    // Initialize systems

    ECS::Registry::Instance().AddSystem<ScreenSystem>();
    ECS::Registry::Instance().AddSystem<GUISystem>();
    ECS::Registry::Instance().AddSystem<RenderTextSystem>();

    //
    // Initialize windows

    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>()
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(std::make_unique<LoggingWindow>()
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(std::make_unique<MemoryWindow>());
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(std::make_unique<FPSWindow>());
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindowControls>()
    );
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance().GetSystem<GUISystem>()
        .GetWindow<LoggingWindow>()
        .SubscribeToEvents();

    ECS::Registry::Instance().Update();

    ECS::Registry::Instance().GetSystem<ScreenSystem>().Update(screen);
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance().GetSystem<GUISystem>().Render(screen, renderer, window);

    // TODO: this doesn't do anything because the text probably needs to be rendered inside one of the GUISystem windows
    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Render(renderer);
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance().GetSystem<GUISystem>().AfterRender();
  }

 private:
  Devices::Screen screen = Devices::Screen(800, 600, 0, 0);
};
