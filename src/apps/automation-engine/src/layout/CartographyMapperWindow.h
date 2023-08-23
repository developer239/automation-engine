#pragma once

#include <vector>
#include "imgui.h"

#include "../systems/CartographySystem.h"
#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class CartographyMapperWindow : public IGUISystemWindow {
 public:
  CartographyMapperWindow(
      CartographySystem& system, std::optional<Devices::Screen>& screen
  )
      : cartographySystem(system), screen(screen) {}

  std::string GetName() override { return "Cartography Mapper"; }

  void Render(Core::Renderer& renderer) override {
    ImGui::Begin(GetName().c_str());

    ROI& region = cartographySystem.regionToScan;

    if (ImGui::Button("Save Mapped as PNG")) {
      SaveMappedAsPNG();
    }

    // Preview
    if (!cartographySystem.captured.empty()) {
      // draw captured
      cvMatrixAsSDLTexture(cartographySystem.captured, renderer);

      ImVec2 windowSize = ImGui::GetWindowSize();

      float scale = std::min(
          windowSize.x / cartographySystem.captured.cols,
          windowSize.y / cartographySystem.captured.rows
      );
      int scaledWidth = cartographySystem.captured.cols * scale;
      int scaledHeight = cartographySystem.captured.rows * scale;

      ImVec2 imageSize = ImVec2(scaledWidth, scaledHeight);
      ImGui::Image(
          (void*)(intptr_t)texture,
          ImVec2(scaledWidth, scaledHeight - 10)
      );
    }

    if (screen) {
      auto screenHeight = *screen->height.get();
      auto screenWidth = *screen->width.get();

      ImGui::SliderInt(
          "X Position",
          &region.location.x,
          0,
          screenWidth - region.size.width
      );
      ImGui::SliderInt(
          "Y Position",
          &region.location.y,
          0,
          screenHeight - region.size.height
      );

      ImGui::SliderInt(
          "Width",
          &region.size.width,
          10,
          screenWidth
      );
      ImGui::SliderInt(
          "Height",
          &region.size.height,
          10,
          screenHeight
      );
    }

    ImGui::End();
  }

  void Clear() { SDL_DestroyTexture(texture); }

 private:
  CartographySystem& cartographySystem;
  std::optional<Devices::Screen>& screen;
  SDL_Texture* texture{};

  void SaveMappedAsPNG() {
    if (!cartographySystem.mapped.empty()) {
      // ../../../../ is project root
      std::string filename = "../../../../mapped.png";
      cv::imwrite(filename, cartographySystem.mapped);
    }
  }

  // TODO: create generic helper
  void cvMatrixAsSDLTexture(cv::Mat& captured, Core::Renderer& renderer) {
    texture = SDL_CreateTexture(
        renderer.Get().get(),
        SDL_PIXELFORMAT_BGR24,
        SDL_TEXTUREACCESS_STREAMING,
        captured.cols,
        captured.rows
    );
    SDL_UpdateTexture(
        texture,
        nullptr,
        (void*)captured.data,
        captured.step1()
    );
  }
};
