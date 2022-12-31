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

#include "../components/PositionComponent.h"
#include "../layout/FPSWindow.h"
#include "../layout/ImageStreamWindow.h"
#include "../layout/MemoryConsumptionWindow.h"
#include "../layout/SidebarTab1Window.h"
#include "../layout/SidebarTab2Window.h"
#include "../layout/Toolbar1Window.h"
#include "../systems/GUISystem/GUISystem.h"
#include "../systems/PositionSystem.h"
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

    ECS::Entity entity = registry.CreateEntity();

    //
    // Initialize components

    registry.AddComponentToEntity<PositionComponent>(entity);

    //
    // Initialize systems

    registry.AddSystem<ScreenSystem>();
    registry.AddSystem<PositionSystem>();
    registry.AddSystem<GUISystem>();

    //
    // Initialize windows

    registry.GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>()
    );
    registry.GetSystem<GUISystem>().AddWindow(
        std::make_unique<SidebarTab1Window>()
    );
    registry.GetSystem<GUISystem>().AddWindow(
        std::make_unique<SidebarTab2Window>()
    );
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<Toolbar1Window>()
    );
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<MemoryWindow>());
    registry.GetSystem<GUISystem>().AddWindow(std::make_unique<FPSWindow>());
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    registry.Update();

    registry.GetSystem<ScreenSystem>().Update(screen);
    registry.GetSystem<PositionSystem>().Update(registry);
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    registry.GetSystem<GUISystem>().Render(screen, renderer);
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {
    registry.GetSystem<GUISystem>().AfterRender();
  }

 private:
  ECS::Registry registry;
  Core::AssetStore assetStore;

  Devices::Screen screen = Devices::Screen(800, 600, 0, 0);
};
