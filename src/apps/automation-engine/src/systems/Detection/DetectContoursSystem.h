#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/DetectContoursComponent.h"
#include "../../components/DetectionComponent.h"

class DetectContoursSystem : public ECS::System {
 public:
  DetectContoursSystem() {
    RequireComponent<DetectionComponent>();
    RequireComponent<DetectContoursComponent>();
  }

  void Update(std::optional<Devices::Screen>& screen) {
    for (auto entity : GetSystemEntities()) {
      auto screenshotDebug = screen->latestScreenshot.clone();
      ApplyOperations(entity, screenshotDebug);
      DetectContours(entity, screenshotDebug);
    }
  }

 private:
  void ApplyOperations(ECS::Entity& entity, cv::Mat& inputMatrix) {
    auto detectionComponent =
        ECS::Registry::Instance().GetComponent<DetectionComponent>(entity);

    for (auto& operation : detectionComponent.operations) {
      operation->Apply(inputMatrix);
    }
  }

  void DetectContours(ECS::Entity& entity, cv::Mat& screenshotDebug) {
    std::vector<std::vector<cv::Point>> contours;

    cv::findContours(
        screenshotDebug,
        contours,
        cv::RETR_TREE,
        cv::CHAIN_APPROX_SIMPLE
    );

    cv::imshow("screenshotDebug", screenshotDebug);

    std::vector<std::vector<cv::Point>> contoursVector(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());
    std::vector<cv::Point2f> centers(contours.size());

    auto contoursComponent =
        ECS::Registry::Instance().GetComponent<DetectContoursComponent>(entity);
    auto group = "detect-contours" + contoursComponent.id;

    for (auto entityToKill :
         ECS::Registry::Instance().GetEntitiesByGroup(group)) {
      ECS::Registry::Instance().KillEntity(entityToKill);
    }

    for (size_t i = 0; i < contours.size(); i++) {
      approxPolyDP(contours[i], contoursVector[i], 10, true);
      auto rect = boundingRect(contours[i]);

      if (rect.width > contoursComponent.minArea.width &&
          rect.height > contoursComponent.minArea.height) {
        if (contoursComponent.maxArea.has_value()) {
          if (rect.width > contoursComponent.maxArea->width ||
              rect.height > contoursComponent.maxArea->height) {
            continue;
          }
        }

        ECS::Entity match = ECS::Registry::Instance().CreateEntity();
        ECS::Registry::Instance().GroupEntity(match, group);

        App::Position offset = {0, 0};

        auto detectionComponent =
            ECS::Registry::Instance().GetComponent<DetectionComponent>(entity);
        for (auto& operation : detectionComponent.operations) {
          auto cropArgs = dynamic_cast<CropOperation*>(operation.get());
          if (cropArgs != nullptr) {
            offset.x = cropArgs->position.x;
            offset.y = cropArgs->position.y;
          }
        }

        int targetX = rect.x + offset.x;
        int targetY = rect.y + offset.y;

        ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
            match,
            App::Position({targetX, targetY}),
            App::Size({rect.width, rect.height}),
            contoursComponent.bboxColor
        );
      }
    }
  }
};
