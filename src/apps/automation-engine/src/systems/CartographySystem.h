#pragma once

#include "../services/Map.h"
#include "../structs/Position.h"
#include "../structs/Size.h"
#include "ScreenSystem.h"
#include "devices/Screen.h"
#include "ecs/System.h"

struct ROI {
  App::Size size;
  App::Position location;
};

class CartographySystem : public ECS::System {
 public:
  bool isMapping = false;
  bool isLocalizing = false;

  // We only want to scan part of the screen area that has distinctive features such as a minimap
  ROI regionToScan = {
      App::Size(1200, 650),
      App::Position(25, 175),

  };

  // we don't want to run template matching on the whole map image, but only on a smaller area around the scannedRegion
  int stitchOuterVisibleOffsetOnMapped = 250;
  // in order for template matching to work we can only use a small part of the scannedRegion
  int stitchInnerOffsetForCrop = 200;

  // cv:Mat that contains image of the scanned region that we want to add to map
  cv::Mat scannedRegion;
  // cv:Mat that contains the stitched image of the scanned regions
  cv::Mat map;

  // x, y coordinates of the regionToScan in the map image
  cv::Point regionLocation;
  // size of the regionToScan in the map image
  App::Size regionLocationSize = App::Size(0, 0);

  explicit CartographySystem(
      std::optional<Devices::Screen>& screen, bool& isRunning
  )
      : screen(screen), isRunning(isRunning){};

  void Update() {
    if (isRunning) {
      scannedRegion = screen->latestScreenshot(cv::Rect(
          regionToScan.location.x,
          regionToScan.location.y,
          regionToScan.size.width,
          regionToScan.size.height
      ));
    }

    if (isRunning && isMapping) {
      if (map.empty()) {
        map = scannedRegion.clone();
        return;
      }

      auto result = stitch(
          map,
          scannedRegion,
          regionLocation,
          stitchOuterVisibleOffsetOnMapped,
          stitchInnerOffsetForCrop
      );

      // TODO: use matchLoc to stitch not scannedRegion but ROI areaToMap or
      // something like that (so that
      // TODO: we can for example use minimap to figure out position and stitch
      // together center of the screen)
      map = result.stitched;
      regionLocation = result.matchLoc;
    }

    if (isLocalizing) {
      performLocalization();
    }
  }

  void performLocalization(bool useWholeImage = false) {
    if (useWholeImage) {
      auto mappedView = map.clone();
      auto capturedView = scannedRegion.clone();
      auto result = templateMatch(mappedView, capturedView);
      regionLocation = result.location;
      regionLocationSize = App::Size(scannedRegion.cols, scannedRegion.rows);
    } else {
      double capturedCropRatio = 1.0;
      double mappedAreaMultiplier = 2.0;

      // 1. Crop the central area from the scannedRegion image based on the given
      // ratio
      int capturedCropWidth = scannedRegion.cols * capturedCropRatio;
      int capturedCropHeight = scannedRegion.rows * capturedCropRatio;
      int capturedOffsetX = (scannedRegion.cols - capturedCropWidth) / 2;
      int capturedOffsetY = (scannedRegion.rows - capturedCropHeight) / 2;

      cv::Mat capturedCropped = scannedRegion(cv::Rect(
          capturedOffsetX,
          capturedOffsetY,
          capturedCropWidth,
          capturedCropHeight
      ));

      // 2. Determine the region in the map image around the regionLocation
      // for template matching. The size is based on the cropped scannedRegion
      // region, but multiplied by mappedAreaMultiplier.
      int mappedAreaWidth = capturedCropped.cols * mappedAreaMultiplier;
      int mappedAreaHeight = capturedCropped.rows * mappedAreaMultiplier;

      // Calculate the offsets for the map area so that the capturedCropped
      // area is roughly in the center
      int mappedOffsetX = (mappedAreaWidth - capturedCropped.cols) / 2;
      int mappedOffsetY = (mappedAreaHeight - capturedCropped.rows) / 2;

      // Adjust for the last location coordinates
      int x = regionLocation.x - mappedOffsetX;
      int y = regionLocation.y - mappedOffsetY;

      // Ensure we don't go out of bounds for the map image
      if (x < 0) x = 0;
      if (y < 0) y = 0;
      if (x + mappedAreaWidth > map.cols) x = map.cols - mappedAreaWidth;
      if (y + mappedAreaHeight > map.rows)
        y = map.rows - mappedAreaHeight;

      cv::Mat mappedSearchRegion =
          map(cv::Rect(x, y, mappedAreaWidth, mappedAreaHeight));

      // 3. Now, perform the template matching
      auto result = templateMatch(mappedSearchRegion, capturedCropped);
      auto normalizedResultLocation =
          cv::Point(result.location.x + x, result.location.y + y);

      // Update last location
      regionLocation = normalizedResultLocation;
      regionLocationSize = App::Size(scannedRegion.cols, scannedRegion.rows);
    }
  }

  void Clear() { SDL_DestroyTexture(texture); }

  ScreenRenderMetadata Render(Core::Renderer& renderer) {
    auto mappedView = map.clone();

    if (isLocalizing) {
      // draw rectangle last location center
      auto markerWidth = 100;
      auto markerHeight = 150;

      auto markerTopLeft = cv::Point(
          regionLocation.x + regionLocationSize.width / 2 - markerWidth / 2,
          regionLocation.y + regionLocationSize.height / 2 - markerHeight / 2
      );

      cv::rectangle(
          mappedView,
          regionLocation,
          cv::Point(
              regionLocation.x + regionLocationSize.width,
              regionLocation.y + regionLocationSize.height
          ),
          cv::Scalar(0, 255, 0),
          5,
          0
      );

      cv::rectangle(
          mappedView,
          markerTopLeft,
          cv::Point(
              markerTopLeft.x + markerWidth,
              markerTopLeft.y + markerHeight
          ),
          cv::Scalar(0, 255, 0),
          5,
          0
      );
    }

    cvMatrixAsSDLTexture(mappedView, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    float scale = std::min(
        windowSize.x / mappedView.cols,
        windowSize.y / mappedView.rows
    );
    int scaledWidth = mappedView.cols * scale;
    int scaledHeight = mappedView.rows * scale;

    ImVec2 imageSize = ImVec2(scaledWidth, scaledHeight);
    auto cursor = ImVec2(
        (windowSize.x - imageSize.x) / 2,
        (windowSize.y - imageSize.y) / 2
    );
    ImGui::SetCursorPos(cursor);
    ImGui::Image(
        (void*)(intptr_t)texture,
        ImVec2(scaledWidth, scaledHeight - 10)
    );

    return {cursor, scale};
  }

 private:
  std::optional<Devices::Screen>& screen;
  bool& isRunning;

  SDL_Texture* texture{};

  // TODO: create generic helper
  void cvMatrixAsSDLTexture(cv::Mat& mapped, Core::Renderer& renderer) {
    texture = SDL_CreateTexture(
        renderer.Get().get(),
        SDL_PIXELFORMAT_BGR24,
        SDL_TEXTUREACCESS_STREAMING,
        mapped.cols,
        mapped.rows
    );
    SDL_UpdateTexture(texture, nullptr, (void*)mapped.data, mapped.step1());
  }
};
