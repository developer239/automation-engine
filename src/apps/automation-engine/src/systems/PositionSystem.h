#pragma once

#include <iostream>

#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../components/PositionComponent.h"

class PositionSystem : public ECS::System {
 public:
  PositionSystem() { RequireComponent<PositionComponent>(); }

  void Update(ECS::Registry& registry) {
    for (auto entity : GetSystemEntities()) {
      auto& component = registry.GetComponent<PositionComponent>(entity);
      component.x += 0.1;
      component.y += 0.1;
    }
  }
};
