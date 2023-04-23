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

  void Render(std::optional<Devices::Screen> screen) {
    if (screen.has_value()) {
      auto imageOutput = screen->latestScreenshot;

      for (auto entity : GetSystemEntities()) {
        auto component =
            ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(entity
            );

        if (component.thickness > 0) {
          cv::rectangle(
              imageOutput,
              {component.position.x, component.position.y},
              {component.position.x + component.size.width,
               component.position.y + component.size.height},
              component.color.ToScalar(),
              component.thickness
          );
        }
      }
    }
  }
};
