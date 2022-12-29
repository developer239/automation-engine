#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"

class ScreenSystem : public ECS::System {
 public:
  ScreenSystem() = default;

  void Update(Devices::Screen screen) { screen.Screenshot(); }
};
