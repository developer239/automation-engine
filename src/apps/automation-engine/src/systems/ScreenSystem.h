#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"

class ScreenSystem : public ECS::System {
 public:
  ScreenSystem() = default;

  void Update(Devices::Screen screen) { screen.Screenshot(); }

  void Render(Devices::Screen& screen, Core::Renderer& renderer) {
    cvMatrixAsSDLTexture(screen, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    float scale = std::min(
        windowSize.x / screen.latestScreenshot.cols,
        windowSize.y / screen.latestScreenshot.rows
    );
    int scaledWidth = screen.latestScreenshot.cols * scale;
    int scaledHeight = screen.latestScreenshot.rows * scale;

    ImVec2 imageSize = ImVec2(scaledWidth, scaledHeight);
    ImGui::SetCursorPos(ImVec2(
        (windowSize.x - imageSize.x) / 2,
        (windowSize.y - imageSize.y) / 2
    ));
    ImGui::Image(
        (void*)(intptr_t)texture,
        ImVec2(scaledWidth, scaledHeight - 10)
    );
  }

  void Clear() { SDL_DestroyTexture(texture); }

 private:
  SDL_Texture* texture{};

  void cvMatrixAsSDLTexture(Devices::Screen& screen, Core::Renderer& renderer) {
    texture = SDL_CreateTexture(
        renderer.Get().get(),
        SDL_PIXELFORMAT_BGR24,
        SDL_TEXTUREACCESS_STREAMING,
        screen.latestScreenshot.cols,
        screen.latestScreenshot.rows
    );
    SDL_UpdateTexture(
        texture,
        nullptr,
        (void*)screen.latestScreenshot.data,
        screen.latestScreenshot.step1()
    );
  }
};
