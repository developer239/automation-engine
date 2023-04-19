#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/DetectContoursComponent.h"
#include "../../components/DetectionComponent.h"
#include "./DetectionSystemBase.h"

class DetectContoursSystem : public DetectionSystemBase {
 public:
  DetectContoursSystem() {
    RequireComponent<DetectionComponent>();
    RequireComponent<DetectContoursComponent>();
  }

  void Update(std::optional<Devices::Screen>& screen) {
    for (auto entity : GetSystemEntities()) {
      auto screenshotDebug = screen->latestScreenshot.clone();
      ApplyOperations(entity, screenshotDebug, screen);
      DetectContours(entity, screenshotDebug, screen);
    }
  }

 private:
  void DetectContours(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto [detectContoursComponent, group] =
        GetComponentAndGroup<DetectContoursComponent>(
            entity,
            "detect-contours"
        );

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    if (screenshotDebug.channels() > 1) {
      cv::cvtColor(screenshotDebug, screenshotDebug, cv::COLOR_BGR2GRAY);
    }

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(
        screenshotDebug,
        contours,
        cv::RETR_TREE,
        cv::CHAIN_APPROX_SIMPLE
    );

    std::vector<std::vector<cv::Point>> contoursVector(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());
    std::vector<cv::Point2f> centers(contours.size());

    for (size_t i = 0; i < contours.size(); i++) {
      approxPolyDP(contours[i], contoursVector[i], 10, true);
      auto rect = boundingRect(contours[i]);

      if (rect.width > detectContoursComponent.minArea.width &&
          rect.height > detectContoursComponent.minArea.height) {
        if (detectContoursComponent.maxArea.has_value()) {
          if (rect.width > detectContoursComponent.maxArea->width ||
              rect.height > detectContoursComponent.maxArea->height) {
            continue;
          }
        }

        ECS::Entity match = ECS::Registry::Instance().CreateEntity();
        ECS::Registry::Instance().GroupEntity(match, group);

        int targetX = rect.x + offset.x;
        int targetY = rect.y + offset.y;

        ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
            match,
            App::Position({targetX, targetY}),
            App::Size({rect.width, rect.height}),
            detectContoursComponent.bboxColor,
            detectContoursComponent.bboxThickness
        );
      }
    }

    // Detection logic END
    //

    if (detectContoursComponent.shouldRenderPreview) {
      RenderPreview(offset, screenshotDebug, screen->latestScreenshot);
    }
  }
};
