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
    // TODO: load from the build folder
    assetStore.AddFont(
        "pico8-font-10",
        "../../../../src/apps/automation-engine/assets/fonts/Roboto-Medium.ttf",
        24
    );

    ECS::Entity ball = registry.CreateEntity();
    registry.TagEntity(ball, "Ball");
    registry.AddComponent<TextLabelComponent>(ball, cv::Vec2i(0, 200), "asdf asdf asd fasf asdf asdfasdf asfda");

    //
    // Initialize systems

    registry.AddSystem<ScreenSystem>();
    registry.AddSystem<GUISystem>();
    registry.AddSystem<RenderTextSystem>();

    //
    // Initialize windows

    registry.GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>()
    );
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<LoggingWindow>()
    );
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<MemoryWindow>());
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<FPSWindow>());
    registry.GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindowControls>()
    );
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    if (eventBus != nullptr) {
      registry.GetSystem<GUISystem>()
          .GetWindow<LoggingWindow>()
          .SubscribeToEvents(eventBus);
    }

    registry.Update();

    registry.GetSystem<ScreenSystem>().Update(screen);
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    registry.GetSystem<GUISystem>().Render(screen, renderer, window);
    // TODO: this doesn't do anything because the text probably needs to be rendered inside one of the GUISystem windows
    registry.GetSystem<RenderTextSystem>().Render(renderer, assetStore, registry);
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {
    registry.GetSystem<GUISystem>().AfterRender();
  }

 private:
  ECS::Registry registry;
  Core::AssetStore assetStore;
  std::unique_ptr<Events::Bus> eventBus = std::make_unique<Events::Bus>();

  Devices::Screen screen = Devices::Screen(800, 600, 0, 0);
};
