#pragma once

#include "ecs/Component.h"

struct PositionComponent : public ECS::Component<PositionComponent> {
  float x = 0;
  float y = 0;
};
