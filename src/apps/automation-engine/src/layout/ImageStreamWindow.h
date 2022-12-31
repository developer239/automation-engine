#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class ImageStreamWindow : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::RIGHT_TOP;
  }

  std::string GetName() override { return "Image Stream"; }

  void Render(const Devices::Screen& screen, Core::Renderer& renderer)
      override {
    cvMatrixAsSDLTexture(screen, renderer);

    ImGui::Image(
        (void*)(intptr_t)texture,
        ImVec2(screen.latestScreenshot.cols, screen.latestScreenshot.rows)
    );
  }

 private:
  SDL_Texture* texture{};

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
};
