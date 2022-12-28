#pragma once

#include "core/IStrategy.h"
#include "core/Renderer.h"
#include "core/Window.h"
#include "ecs/Component.h"
#include "ecs/Entity.h"
#include "ecs/Registry.h"
#include "ecs/System.h"
#include "../components/PositionComponent.h"
#include "../systems/PositionSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Entity entity = registry.CreateEntity();

    registry.AddComponentToEntity<PositionComponent>(entity);
    registry.AddSystem<PositionSystem>();
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    registry.Update();
    registry.GetSystem<PositionSystem>().Update(registry);
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {}

 private:
  ECS::Registry registry;
};
