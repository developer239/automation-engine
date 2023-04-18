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
      ApplyOperations(entity, screenshotDebug, screen);
      DetectContours(entity, screenshotDebug, screen);
    }
  }

 private:
  // Note: possibly generalize
  void ApplyOperations(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto detectionComponent =
        ECS::Registry::Instance().GetComponent<DetectionComponent>(entity);

    for (auto& operation : detectionComponent.operations) {
      FixCropOperation(operation, screen);

      operation->Apply(screenshotDebug);
    }
  }

  // Note: possibly generalize
  void FixCropOperation(std::shared_ptr<Operation> operation, std::optional<Devices::Screen>& screen) {
    auto cropArgs = dynamic_cast<CropOperation*>(operation.get());
    if (cropArgs != nullptr) {
      if (cropArgs->position.x + cropArgs->size.width > *screen->width) {
        auto prevWidth = cropArgs->size.width;
        auto targetWidth = *screen->width - cropArgs->position.x;

        cropArgs->position.x -= prevWidth - targetWidth;
        if (cropArgs->size.width + cropArgs->position.x > *screen->width) {
          cropArgs->size.width = *screen->width - cropArgs->position.x;
        }

        if (cropArgs->position.x < 0) {
          cropArgs->position.x = 0;
          cropArgs->size.width = *screen->width - 1;
        }
      }

      if (cropArgs->position.y + cropArgs->size.height > *screen->height) {
        auto prevHeight = cropArgs->size.height;
        auto targetHeight = *screen->height - cropArgs->position.y;

        cropArgs->position.y -= prevHeight - targetHeight;
        if (cropArgs->size.height + cropArgs->position.y > *screen->height) {
          cropArgs->size.height = *screen->height - cropArgs->position.y;
        }

        if (cropArgs->position.y < 0) {
          cropArgs->position.y = 0;
          cropArgs->size.height = *screen->height - 1;
        }
      }
    }
  }

  // Note: possibly generalize
  void RenderPreview(
      App::Position offset, cv::Mat screenshotDebug, cv::Mat& outputScreenshot
  ) {
    if (screenshotDebug.channels() == 1) {
      cv::cvtColor(screenshotDebug, screenshotDebug, cv::COLOR_GRAY2BGR);
    }

    for (auto row = 0; row < screenshotDebug.rows; row++) {
      for (auto col = 0; col < screenshotDebug.cols; col++) {
        int targetRow = row + offset.y;
        int targetCol = col + offset.x;

        outputScreenshot.at<cv::Vec3b>(targetRow, targetCol) =
            screenshotDebug.at<cv::Vec3b>(row, col);
      }
    }
  }

  void DetectContours(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    std::vector<std::vector<cv::Point>> contours;

    if (screenshotDebug.channels() > 1) {
      cv::cvtColor(screenshotDebug, screenshotDebug, cv::COLOR_BGR2GRAY);
    }

    cv::findContours(
        screenshotDebug,
        contours,
        cv::RETR_TREE,
        cv::CHAIN_APPROX_SIMPLE
    );

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

    App::Position offset = {0, 0};
    offset.x += *screen->windowX;
    offset.y += *screen->windowY;

    auto detectionComponent =
        ECS::Registry::Instance().GetComponent<DetectionComponent>(entity);
    for (auto& operation : detectionComponent.operations) {
      auto cropArgs = dynamic_cast<CropOperation*>(operation.get());
      if (cropArgs != nullptr) {
        offset.x = cropArgs->position.x;
        offset.y = cropArgs->position.y;
      }
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

    if (contoursComponent.shouldRenderPreview) {
      RenderPreview(offset, screenshotDebug, screen->latestScreenshot);
    }
  }
};
