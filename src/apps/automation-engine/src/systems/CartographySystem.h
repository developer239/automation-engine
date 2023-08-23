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
  ROI regionToScan = {
      App::Size(115, 90),
      App::Position(1218, 100),

  };
  cv::Mat captured;
  cv::Mat mapped;
  cv::Point lastLocation;

  explicit CartographySystem(
      std::optional<Devices::Screen>& screen, bool& isRunning
  )
      : screen(screen), isRunning(isRunning){};

  void Update() {
    // TODO: only map if isRunning and isMapping
    if (isRunning) {
      captured = screen->latestScreenshot(cv::Rect(
          regionToScan.location.x,
          regionToScan.location.y,
          regionToScan.size.width,
          regionToScan.size.height
      ));

      if (mapped.empty()) {
        mapped = captured.clone();
        return;
      }

      auto result = stitch(mapped, captured, lastLocation);

      // TODO: use matchLoc to stitch not captured but ROI areaToMap or something like that (so that
      // TODO: we can for example use minimap to figure out position and stitch together center of the screen)
      mapped = result.stitched;
      lastLocation = result.matchLoc;
    }
  }

  void Clear() { SDL_DestroyTexture(texture); }

  ScreenRenderMetadata Render(
      Core::Renderer& renderer
  ) {
    cvMatrixAsSDLTexture(mapped, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    float scale = std::min(
        windowSize.x / mapped.cols,
        windowSize.y / mapped.rows
    );
    int scaledWidth = mapped.cols * scale;
    int scaledHeight = mapped.rows * scale;

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
    SDL_UpdateTexture(
        texture,
        nullptr,
        (void*)mapped.data,
        mapped.step1()
    );
  }
};
