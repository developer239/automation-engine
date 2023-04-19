#pragma once

#undef fract1  // https://stackoverflow.com/questions/36394155/leptonica-conflicts-with-xcode-framework

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include "devices/Screen.h"
#include "ecs/System.h"

#include "../../components/DetectTextComponent.h"
#include "../../components/DetectionComponent.h"
#include "./DetectionSystemBase.h"

struct PixDeleter {
  void operator()(Pix* pix) const {
    pixDestroy(&pix);
  }
};

using PixPtr = std::unique_ptr<Pix, PixDeleter>;

class DetectTextSystem : public DetectionSystemBase {
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

  ~DetectTextSystem() { api->End(); }

  void Update(std::optional<Devices::Screen>& screen) {
    for (auto entity : GetSystemEntities()) {
      auto screenshotDebug = screen->latestScreenshot.clone();
      ApplyOperations(entity, screenshotDebug, screen);
      DetectText(entity, screenshotDebug, screen);
    }
  }

 private:
  void DetectText(
      ECS::Entity& entity, cv::Mat& screenshotDebug,
      std::optional<Devices::Screen>& screen
  ) {
    auto [detectTextComponent, group] =
        GetComponentAndGroup<DetectTextComponent>(entity, "detect-text");

    KillComponentsInGroup(group);

    App::Position offset = CalculateMatchOffset(entity);

    //
    // Detection logic START

    PixPtr pixImage = MatToPix(&screenshotDebug);
    api->SetImage(pixImage.get());
    auto text = api->GetUTF8Text();

    ECS::Entity match = ECS::Registry::Instance().CreateEntity();
    ECS::Registry::Instance().GroupEntity(match, group);

    ECS::Registry::Instance().AddComponent<TextLabelComponent>(
        match,
        text,
        App::Position({offset.x, offset.y}),
        detectTextComponent.bboxColor
    );

    // Detection logic END
    //

    if (detectTextComponent.shouldRenderPreview) {
      RenderPreview(offset, screenshotDebug, screen->latestScreenshot);
    }
  }

  PixPtr MatToPix(cv::Mat* mat) {
    Pix* pixd = pixCreate(mat->size().width, mat->size().height, 8);
    for (int y = 0; y < mat->rows; y++) {
      for (int x = 0; x < mat->cols; x++) {
        pixSetPixel(pixd, x, y, (l_uint32)mat->at<uchar>(y, x));
      }
    }
    return PixPtr(pixd);
  }
};
