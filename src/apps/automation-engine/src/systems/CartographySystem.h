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

  ROI regionToScan = {
      App::Size(1200, 650),
      App::Position(25, 175),

  };
  int stitchOffset = 250;
  int stitchMoveByCrop = 50;
  cv::Mat captured;
  cv::Mat mapped;

  bool isLocalizing = false;
  cv::Point lastLocation;
  App::Size lastLocationRegion = App::Size(0, 0);

  explicit CartographySystem(
      std::optional<Devices::Screen>& screen, bool& isRunning
  )
      : screen(screen), isRunning(isRunning){};

  void Update() {
    if(isRunning) {
      captured = screen->latestScreenshot(cv::Rect(
          regionToScan.location.x,
          regionToScan.location.y,
          regionToScan.size.width,
          regionToScan.size.height
      ));
    }

    if (isRunning && isMapping) {
      if (mapped.empty()) {
        mapped = captured.clone();
        return;
      }

      auto result = stitch(mapped, captured, lastLocation, stitchOffset, stitchMoveByCrop);

      // TODO: use matchLoc to stitch not captured but ROI areaToMap or
      // something like that (so that
      // TODO: we can for example use minimap to figure out position and stitch
      // together center of the screen)
      mapped = result.stitched;
      lastLocation = result.matchLoc;
    }

    if (isLocalizing) {
      auto mappedView = mapped.clone();
      auto capturedView = captured.clone();
      // TODO: figure out why does template match convert colors aggressively
      auto result = templateMatch(mappedView, capturedView);
      lastLocation = result.location;
      lastLocationRegion = App::Size(captured.cols, captured.rows);
    }
  }

  void Clear() { SDL_DestroyTexture(texture); }

  ScreenRenderMetadata Render(Core::Renderer& renderer) {
    auto mappedView = mapped.clone();

    if (isLocalizing) {
      cv::rectangle(
          mappedView,
          lastLocation,
          cv::Point(
              lastLocation.x + lastLocationRegion.width,
              lastLocation.y + lastLocationRegion.height
          ),
          cv::Scalar(0, 255, 0),
          2,
          8
      );
    }

    cvMatrixAsSDLTexture(mappedView, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    float scale =
        std::min(windowSize.x / mappedView.cols, windowSize.y / mappedView.rows);
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
