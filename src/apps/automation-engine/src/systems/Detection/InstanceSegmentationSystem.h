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
      InstanceSegmentation(entity, screen->latestScreenshot, screen);
    }
  }

 private:
  // TODO: sometimes crashes the app
  // libc++abi: terminating with uncaught exception of type cv::Exception:
  // OpenCV(4.7.0)
  // /tmp/opencv-20230122-99400-19ne6ip/opencv-4.7.0/modules/core/src/matrix.cpp:809:
  // error: (-215:Assertion failed) 0 <= roi.x && 0 <= roi.width && roi.x +
  // roi.width <= m.cols && 0 <= roi.y && 0 <= roi.height && roi.y + roi.height
  // <= m.rows in function 'Mat'
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

      int targetX = segment.bbox.x + offset.x;
      int targetY = segment.bbox.y + offset.y;

      auto randomColor = App::Color({rand() % 255, rand() % 255, rand() % 255});

      ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
          match,
          App::Position({targetX, targetY}),
          App::Size({segment.bbox.width, segment.bbox.height}),
          randomColor,
          3
      );
    }

    // Detection logic END
    //
  }
};
