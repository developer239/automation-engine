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

  void Render(Devices::Screen& screen, Core::Renderer& renderer)
      override {
    ImGui::Begin(GetName().c_str());
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
    ImGui::End();
  }

  void Clear() override { SDL_DestroyTexture(texture); }

 private:
  SDL_Texture* texture{};

  void cvMatrixAsSDLTexture(
      Devices::Screen& screen, Core::Renderer& renderer
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
