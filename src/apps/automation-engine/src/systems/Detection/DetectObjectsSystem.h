#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"
#include "yolo/YOLODetector.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/DetectObjectsComponent.h"
#include "../../components/DetectionComponent.h"
#include "./DetectionSystemBase.h"

class DetectObjectsSystem : public DetectionSystemBase {
 public:
  DetectObjectsSystem() {
    RequireComponent<DetectionComponent>();
    RequireComponent<DetectObjectsComponent>();
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
    auto [detectObjectsComponent, group] =
        GetComponentAndGroup<DetectObjectsComponent>(entity, "detect-objects");

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    detectObjectsComponent.net->detect(screenshotDebug);

    // Detection logic END
    //
  }
};
