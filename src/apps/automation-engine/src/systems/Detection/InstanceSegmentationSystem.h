#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"
#include "yolo/YOLODetector.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/InstanceSegmentationComponent.h"
#include "../../components/SegmentMaskComponent.h"
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
      InstanceSegmentation(entity, screenshotDebug, screen);
    }
  }

 private:
  std::map<int, App::Color> idColorMap;

  void InstanceSegmentation(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto [instanceSegmentationComponent, group] =
        GetComponentAndGroup<InstanceSegmentationComponent>(
            entity,
            "segment-image"
        );

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    auto segments = instanceSegmentationComponent.net->Detect(screenshotDebug);

    for (auto segment : segments) {
      auto match = ECS::Registry::Instance().CreateEntity();
      ECS::Registry::Instance().GroupEntity(match, group);

      segment.bbox.x += offset.x;
      segment.bbox.y += offset.y;

      if (idColorMap.find(segment.id) == idColorMap.end()) {
        idColorMap.insert(
            {segment.id, App::Color({rand() % 255, rand() % 255, rand() % 255})}
        );
      }
      auto color = idColorMap.at(segment.id);

      ECS::Registry::Instance().AddComponent<SegmentMaskComponent>(
          match,
          color,
          segment.bbox,
          segment.mask
      );
    }

    // Detection logic END
    //
  }
};
