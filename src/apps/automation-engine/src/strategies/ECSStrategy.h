#pragma once

#include "core/IStrategy.h"
#include "core/Renderer.h"
#include "core/AssetStore.h"
#include "core/Window.h"
#include "devices/Screen.h"
#include "ecs/Component.h"
#include "ecs/Entity.h"
#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../components/PositionComponent.h"
#include "../systems/GUISystem.h"
#include "../systems/PositionSystem.h"
#include "../systems/ScreenSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    // TODO: load from the build folder
    assetStore.AddFont("pico8-font-10", "../../../../src/apps/automation-engine/assets/fonts/Roboto-Medium.ttf", 24);

    ECS::Entity entity = registry.CreateEntity();

    registry.AddComponentToEntity<PositionComponent>(entity);

    registry.AddSystem<ScreenSystem>();
    registry.AddSystem<PositionSystem>();
    registry.AddSystem<GUISystem>();
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    registry.Update();

    registry.GetSystem<ScreenSystem>().Update(screen);
    registry.GetSystem<PositionSystem>().Update(registry);
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    registry.GetSystem<GUISystem>().Render();
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {}

 private:
  ECS::Registry registry;
  Core::AssetStore assetStore;

  Devices::Screen screen = Devices::Screen(800, 600, 0, 0);
};
