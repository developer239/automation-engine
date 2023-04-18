#pragma once

#undef fract1  // https://stackoverflow.com/questions/36394155/leptonica-conflicts-with-xcode-framework

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include "devices/Screen.h"
#include "ecs/System.h"

#include "../../components/BoundingBoxComponent.h"
#include "../../components/DetectContoursComponent.h"
#include "../../components/DetectTextComponent.h"
#include "../../components/DetectionComponent.h"

class DetectTextSystem : public ECS::System {
 public:
  std::unique_ptr<tesseract::TessBaseAPI> api;

  DetectTextSystem() {
    RequireComponent<DetectionComponent>();
    RequireComponent<DetectTextComponent>();

    api = std::make_unique<tesseract::TessBaseAPI>();

    if (api->Init(nullptr, "eng")) {
      fprintf(stderr, "Could not initialize Tesseract.");
      exit(1);
    };
  }

  ~DetectTextSystem() {
    api->End();
  }

  void Update(std::optional<Devices::Screen>& screen) {
    for (auto entity : GetSystemEntities()) {
      auto screenshotDebug = screen->latestScreenshot.clone();
      ApplyOperations(entity, screenshotDebug, screen);
      DetectText(entity, screenshotDebug, screen);
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

  Pix* MatToPix(cv::Mat* mat) {
    Pix* pixd = pixCreate(mat->size().width, mat->size().height, 8);
    for (int y = 0; y < mat->rows; y++) {
      for (int x = 0; x < mat->cols; x++) {
        pixSetPixel(pixd, x, y, (l_uint32)mat->at<uchar>(y, x));
      }
    }

    return pixd;
  }

  void DetectText(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto detectTextComponent =
        ECS::Registry::Instance().GetComponent<DetectTextComponent>(entity);
    auto group = "detect-text" + detectTextComponent.id;

    for (auto entityToKill :
         ECS::Registry::Instance().GetEntitiesByGroup(group)) {
      ECS::Registry::Instance().KillEntity(entityToKill);
    }

    // note generalise
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

    auto pixImage = MatToPix(&screenshotDebug);
    api->SetImage(pixImage);
    auto text = api->GetUTF8Text();

    ECS::Entity match = ECS::Registry::Instance().CreateEntity();
    ECS::Registry::Instance().GroupEntity(match, group);

    ECS::Registry::Instance().AddComponent<BoundingBoxComponent>(
        match,
        // TODO: calculate position
        App::Position({offset.x, offset.y}),
        // TODO: calculate size
        App::Size({200, 200}),
        detectTextComponent.bboxColor,
        detectTextComponent.bboxThickness
    );
    ECS::Registry::Instance().AddComponent<TextLabelComponent>(
        match,
        text,
        // TODO: calculate position
        App::Position({offset.x, offset.y}),
        detectTextComponent.bboxColor
    );

    if (detectTextComponent.shouldRenderPreview) {
      RenderPreview(offset, screenshotDebug, screen->latestScreenshot);
    }

    pixDestroy(&pixImage);
  }
};
