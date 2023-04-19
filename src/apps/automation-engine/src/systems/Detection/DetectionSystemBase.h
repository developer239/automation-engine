#pragma once

#include "devices/Screen.h"
#include "ecs/Entity.h"
#include "ecs/Registry.h"
#include "ecs/System.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/DetectionComponent.h"

class DetectionSystemBase : public ECS::System {
 protected:
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

  void FixCropOperation(
      std::shared_ptr<Operation> operation,
      std::optional<Devices::Screen>& screen
  ) {
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

  template<typename TComponent>
  std::pair<TComponent, std::string> GetComponentAndGroup(ECS::Entity& entity, std::string prefix) {
    TComponent& component = ECS::Registry::Instance().GetComponent<TComponent>(entity);
    std::string group = prefix + component.id;
    return {component, group};
  }

  void KillComponentsInGroup(std::string groupName) {
    for (auto entityToKill :
         ECS::Registry::Instance().GetEntitiesByGroup(groupName)) {
      ECS::Registry::Instance().KillEntity(entityToKill);
    }
  }

  App::Position CalculateMatchOffset(ECS::Entity entity) {
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

    return offset;
  }
};
