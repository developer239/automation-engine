#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"

struct ScreenRenderMetadata {
  ImVec2 cursor;
  float scale;
};

class ScreenSystem : public ECS::System {
 public:
  ScreenSystem() = default;

  void Update(std::optional<Devices::Screen> screen) { screen->Screenshot(); }

  ScreenRenderMetadata Render(
      std::optional<Devices::Screen> screen, Core::Renderer& renderer
  ) {
    cvMatrixAsSDLTexture(screen, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    float scale = std::min(
        windowSize.x / screen->latestScreenshot.cols,
        windowSize.y / screen->latestScreenshot.rows
    );
    int scaledWidth = screen->latestScreenshot.cols * scale;
    int scaledHeight = screen->latestScreenshot.rows * scale;

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

  void Clear() { SDL_DestroyTexture(texture); }

 private:
  // TODO: support multiple textures 🤦‍♂️
  SDL_Texture* texture{};

  // TODO: create generic helper
  void cvMatrixAsSDLTexture(std::optional<Devices::Screen> screen, Core::Renderer& renderer) {
    texture = SDL_CreateTexture(
        renderer.Get().get(),
        SDL_PIXELFORMAT_BGR24,
        SDL_TEXTUREACCESS_STREAMING,
        screen->latestScreenshot.cols,
        screen->latestScreenshot.rows
    );
    SDL_UpdateTexture(
        texture,
        nullptr,
        (void*)screen->latestScreenshot.data,
        screen->latestScreenshot.step1()
    );
  }
};
