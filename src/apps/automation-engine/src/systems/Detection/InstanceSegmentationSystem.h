#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"
#include "yolo/YOLODetector.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/InstanceSegmentationComponent.h"
#include "./DetectionSystemBase.h"

class InstanceSegmentationSystem : public DetectionSystemBase {
 public:
  InstanceSegmentationSystem() {
    RequireComponent<DetectionComponent>();
    RequireComponent<InstanceSegmentationComponent>();
  }

  void Update(std::optional<Devices::Screen>& screen) {
    for (auto entity : GetSystemEntities()) {
      auto screenshotDebug = screen->latestScreenshot.clone();
      ApplyOperations(entity, screenshotDebug, screen);
      DetectObjects(entity, screen->latestScreenshot, screen);
    }
  }

 private:
  void DetectObjects(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto [imageInstanceSegmentationComponent, group] =
        GetComponentAndGroup<InstanceSegmentationComponent>(entity, "segment-image");

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    imageInstanceSegmentationComponent.net->Detect(screenshotDebug);

    // Detection logic END
    //
  }
};
