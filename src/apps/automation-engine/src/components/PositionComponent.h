#pragma once

struct PositionComponent : public ECS::Component<PositionComponent> {
  float x = 0;
  float y = 0;
};
