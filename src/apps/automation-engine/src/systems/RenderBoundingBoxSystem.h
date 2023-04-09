#pragma once

#include <SDL.h>

#include "core/Renderer.h"
#include "devices/Screen.h"
#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../components/BoundingBoxComponent.h"

class RenderBoundingBoxSystem : public ECS::System {
 public:
  RenderBoundingBoxSystem() { RequireComponent<BoundingBoxComponent>(); }

  void Render(Devices::Screen& screen) {
    auto imageOutput = screen.latestScreenshot;

    for (auto entity : GetSystemEntities()) {
      auto component =
          ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(entity);

      cv::rectangle(
          imageOutput,
          {component.positionX, component.positionY},
          {component.positionX + component.width,
           component.positionY + component.height},
          component.color,
          3
      );
    }
  }
};
