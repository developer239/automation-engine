#pragma once

#include <vector>
#include "imgui.h"

#include "../../../../../externals/ImGuiFileDialog/ImGuiFileDialog.h"
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

    if (ImGui::Button("Find path")) {
      cartographySystem.findPath();
    }
    if (cartographySystem.isNavigating) {
      ImGui::Text("Navigating");
      if (ImGui::Button("Stop navigating")) {
        cartographySystem.isNavigating = !cartographySystem.isNavigating;
      }
    } else {
      ImGui::Text("Not navigating");
      if (ImGui::Button("Start navigating")) {
        cartographySystem.isNavigating = !cartographySystem.isNavigating;
      }
    }

    if (ImGui::Button("Open Map File Dialog")) {
      ImGuiFileDialog::Instance()->OpenDialog(
          "ChooseFileDlgKeyMap",
          "Choose File",
          ".png",
          "../../../../"
      );
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKeyMap")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        auto filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

        cartographySystem.map = cv::imread(filePathName);
        cartographySystem.walkableMask = cv::Mat(
            cartographySystem.map.rows,
            cartographySystem.map.cols,
            CV_8UC1,
            cv::Scalar(0)
        );
      }

      ImGuiFileDialog::Instance()->Close();
    }

//    if (ImGui::Button("Save Mapped as PNG")) {
//      SaveMappedAsPNG();
//    }

    if (ImGui::Button("Open Walkable File Dialog")) {
      ImGuiFileDialog::Instance()->OpenDialog(
          "ChooseFileDlgKeyWalkable",
          "Choose File",
          ".png",
          "../../../../"
      );
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKeyWalkable")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        auto filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

        auto maskRBG = cv::imread(filePathName);
        auto maskCV_8UC1 = cv::Mat(
            maskRBG.rows,
            maskRBG.cols,
            CV_8UC1,
            cv::Scalar(0)
        );
        cv::cvtColor(maskRBG, maskCV_8UC1, cv::COLOR_BGR2GRAY);
        cartographySystem.walkableMask = maskCV_8UC1;
      }

      ImGuiFileDialog::Instance()->Close();
    }

//    if (ImGui::Button("Save Walkable as PNG")) {
//      SaveWalkableAreaAsPNG();
//    }

    if (ImGui::Button("Clear Map")) {
      cartographySystem.map = cv::Mat();
      cartographySystem.regionLocation = cv::Point();
      cartographySystem.regionLocationSize = App::Size(0, 0);
    }

    if (ImGui::Button("Clear Location")) {
      cartographySystem.regionLocation = cv::Point();
    }
    if (ImGui::Button("Localize on whole image")) {
      cartographySystem.performLocalization(true);
    }

    ImGui::SliderInt(
        "Stitch offset",
        &cartographySystem.stitchOuterVisibleOffsetOnMapped,
        0,
        300
    );
    ImGui::SliderInt(
        "Stitch Move By Crop",
        &cartographySystem.stitchInnerOffsetForCrop,
        0,
        200
    );

    if (cartographySystem.isLocalizing) {
      ImGui::Text("Localizing");
      if (ImGui::Button("Stop localizing")) {
        cartographySystem.isLocalizing = !cartographySystem.isLocalizing;
      }
    } else {
      ImGui::Text("Not localizing");
      if (ImGui::Button("Start localizing")) {
        cartographySystem.isLocalizing = !cartographySystem.isLocalizing;
      }
    }

    if (cartographySystem.isGeneratingWalkableArea) {
      ImGui::Text("Is generating walkable area");
      if (ImGui::Button("Stop generating walkable area")) {
        cartographySystem.isGeneratingWalkableArea = !cartographySystem.isGeneratingWalkableArea;
      }
    } else {
      ImGui::Text("Not generating walkable area");
      if (ImGui::Button("Start generating walkable area")) {
        cartographySystem.isGeneratingWalkableArea = !cartographySystem.isGeneratingWalkableArea;
      }
    }

    if (cartographySystem.isShowingWalkableArea) {
      ImGui::Text("Is showing walkable area");
      if (ImGui::Button("Stop showing walkable area")) {
        cartographySystem.isShowingWalkableArea = !cartographySystem.isShowingWalkableArea;
      }
    } else {
      ImGui::Text("Not showing walkable area");
      if (ImGui::Button("Start showing walkable area")) {
        cartographySystem.isShowingWalkableArea = !cartographySystem.isShowingWalkableArea;
      }
    }

    if (cartographySystem.isMapping) {
      ImGui::Text("Mapping");
      if (ImGui::Button("Stop mapping")) {
        cartographySystem.isMapping = !cartographySystem.isMapping;
      }
    } else {
      ImGui::Text("Not Mapping");
      if (ImGui::Button("Start mapping")) {
        cartographySystem.isMapping = !cartographySystem.isMapping;
      }
    }

    // Preview
    if (!cartographySystem.scannedRegion.empty()) {
      // draw scannedRegion
      cvMatrixAsSDLTexture(cartographySystem.scannedRegion, renderer);

      ImVec2 windowSize = ImGui::GetWindowSize();

      float scale = std::min(
          windowSize.x / cartographySystem.scannedRegion.cols,
          windowSize.y / cartographySystem.scannedRegion.rows
      );
      int scaledWidth = cartographySystem.scannedRegion.cols * scale;
      int scaledHeight = cartographySystem.scannedRegion.rows * scale;

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

      ImGui::SliderInt("Width", &region.size.width, 10, screenWidth);
      ImGui::SliderInt("Height", &region.size.height, 10, screenHeight);
    }

    ImGui::End();
  }

  void Clear() { SDL_DestroyTexture(texture); }

 private:
  CartographySystem& cartographySystem;
  std::optional<Devices::Screen>& screen;
  SDL_Texture* texture{};

  void SaveMappedAsPNG() {
    if (!cartographySystem.map.empty()) {
      // ../../../../ is project root
      std::string filename = "../../../../map.png";
      cv::imwrite(filename, cartographySystem.map);
    }
  }

  void SaveWalkableAreaAsPNG() {
    if (!cartographySystem.map.empty()) {
      // ../../../../ is project root
      std::string filename = "../../../../walkable.png";
      cv::imwrite(filename, cartographySystem.walkableMask);
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
    SDL_UpdateTexture(texture, nullptr, (void*)captured.data, captured.step1());
  }
};
