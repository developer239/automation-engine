#pragma once

#include <SDL.h>

#include "core/Renderer.h"
#include "devices/Screen.h"
#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../components/SegmentMaskComponent.h"

class RenderSegmentMaskSystem : public ECS::System {
 public:
  RenderSegmentMaskSystem() { RequireComponent<SegmentMaskComponent>(); }

  void Render(std::optional<Devices::Screen> screen) {
    if (screen.has_value()) {
      auto& imageOutput = screen->latestScreenshot;

      cv::Mat imageWithMask = screen->latestScreenshot.clone();
      for (auto entity : GetSystemEntities()) {
        auto component =
            ECS::Registry::Instance().GetComponent<SegmentMaskComponent>(entity
            );

        if (component.shouldDrawBbox) {
          rectangle(
              imageOutput,
              component.bbox,
              component.color.ToScalar(),
              2,
              8
          );
        }

        // TODO: prevent error (happens when objects out of screen?)
        try {
          if (component.shouldDrawMask) {
            imageWithMask(component.bbox)
                .setTo(component.color.ToScalar(), component.mask);
          }

        } catch (cv::Exception& e) {
          std::cout << "Error: " << e.what() << std::endl;
        }
      }

      addWeighted(imageWithMask, 0.5, imageOutput, 0.5, 0, imageOutput);
    }
  }
};
