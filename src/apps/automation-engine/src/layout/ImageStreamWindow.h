#pragma once

#include "imgui.h"

#include "core/Renderer.h"
#include "devices/Screen.h"

class ImageStreamWindow {
 public:
  ~ImageStreamWindow() { SDL_DestroyTexture(texture); }

  void Render(const Devices::Screen& screen, Core::Renderer& renderer) {
    cvMatrixAsSDLTexture(screen, renderer);

    ImGui::Image(
        (void*)(intptr_t)texture,
        ImVec2(screen.latestScreenshot.cols, screen.latestScreenshot.rows)
    );
  }

  void cvMatrixAsSDLTexture(
      const Devices::Screen& screen, Core::Renderer& renderer
  ) {
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

 private:
  SDL_Texture* texture;
};
