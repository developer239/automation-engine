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

      ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
          match,
          App::Position(segment.bbox.x, segment.bbox.y),
          App::Size(segment.bbox.width, segment.bbox.height),
          color,
          0
      );
      ECS::Registry::Instance().AddComponent<SegmentMaskComponent>(
          match,
          color,
          segment.bbox,
          segment.mask,
          true
      );
      ECS::Registry::Instance().GroupEntity(match, segment.labelName);
    }

    // Detection logic END
    //
  }

  // TODO: optimize and inpaint once not for every segment
  void inPaint(
      std::optional<Devices::Screen>& screen, cv::Mat mask, cv::Rect bbox
  ) {
    if (!screen || screen->latestScreenshot.empty()) {
      std::cerr << "Error: Screen not provided or latestScreenshot is empty!"
                << std::endl;
      return;
    }

    cv::Mat fullSizeMask =
        cv::Mat::zeros(screen->latestScreenshot.size(), mask.type());

    cv::Mat dilatedMask;
    int dilationSize = 4;
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1),
        cv::Point(dilationSize, dilationSize)
    );
    cv::dilate(mask, dilatedMask, kernel);

    if (bbox.size() != dilatedMask.size()) {
      std::cerr << "Error: bbox size and dilatedMask size do not match!"
                << std::endl;
      return;
    }

    dilatedMask.copyTo(fullSizeMask(bbox));

    cv::inpaint(
        screen->latestScreenshot,
        fullSizeMask,
        screen->latestScreenshot,
        4,
        cv::INPAINT_TELEA
    );
  }
};
