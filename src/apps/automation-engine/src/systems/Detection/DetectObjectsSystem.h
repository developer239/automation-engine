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
      DetectObjects(entity, screenshotDebug);
    }
  }

 private:
  std::map<int, App::Color> idColorMap;

  void DetectObjects(
      ECS::Entity& entity, cv::Mat& screenshotDebug
  ) {
    auto [detectObjectsComponent, group] =
        GetComponentAndGroup<DetectObjectsComponent>(entity, "detect-objects");

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    auto boxes = detectObjectsComponent.net->Detect(screenshotDebug);

    for (auto box : boxes) {
      auto match = ECS::Registry::Instance().CreateEntity();
      ECS::Registry::Instance().GroupEntity(match, group);

      int targetX = box.x1 + offset.x;
      int targetY = box.y1 + offset.y;

      if (idColorMap.find(box.label) == idColorMap.end()) {
        idColorMap.insert(
            {box.label, App::Color({rand() % 255, rand() % 255, rand() % 255})}
        );
      }
      auto color = idColorMap.at(box.label);

      ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
          match,
          App::Position({targetX, targetY}),
          App::Size({(int)(box.x2-box.x1), (int)(box.y2-box.y1)}),
          color,
          3
      );
      ECS::Registry::Instance().GroupEntity(match, box.labelName);
    }

    // Detection logic END
    //
  }
};
