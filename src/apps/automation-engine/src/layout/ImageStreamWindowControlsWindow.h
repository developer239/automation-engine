#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class PreviewRectangle {
 public:
  float previewWidth = 0;
  float previewHeight = 0;
  float scale = 1;
  float x = 0;
  float y = 0;

  PreviewRectangle(SDL_Color textureColor, SDL_Color textColor)
      : sdlTextureColor(textureColor), sdlTextColor(textColor) {}

  void UpdateData(
      int originalWidth, int originalHeight,
      // if lives inside parent use to pass parent's area scale
      float scale = 0
  ) {
    this->originalWidth = originalWidth;
    this->originalHeight = originalHeight;

    if (!scale) {
      ImVec2 regionSize = ImGui::GetContentRegionAvail();
      auto widthRatio = regionSize.x / originalWidth;
      auto heightRatio = regionSize.y / originalHeight;
      this->scale = std::min(widthRatio, heightRatio);
    } else {
      this->scale = scale;
    }

    this->previewWidth = this->originalWidth * this->scale;
    this->previewHeight = this->originalHeight * this->scale;
    this->label =
        std::to_string(originalWidth) + "x" + std::to_string(originalHeight);
  }

  void RenderSDLTexture(Core::Renderer& renderer) {
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0,
        originalWidth * scale,
        originalHeight * scale,
        32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000
    );
    SDL_FillRect(
        surface,
        nullptr,
        SDL_MapRGB(
            surface->format,
            sdlTextureColor.r,
            sdlTextureColor.g,
            sdlTextureColor.b
        )
    );

    SDL_Surface* textSurface =
        TTF_RenderText_Solid(font.get(), label.c_str(), sdlTextColor);

    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
    SDL_BlitSurface(textSurface, nullptr, surface, &textRect);
    sdlTexture = SDL_CreateTextureFromSurface(renderer.Get().get(), surface);

    SDL_FreeSurface(surface);
    SDL_FreeSurface(textSurface);
  }

  void RenderImGuiTexture() {
    ImGui::Image(sdlTexture, {previewWidth, previewHeight});
  }

  void ClearTexture() { SDL_DestroyTexture(sdlTexture); }

 private:
  SDL_Texture* sdlTexture = nullptr;

  int originalWidth = 0;
  int originalHeight = 0;

  std::string label;

  SDL_Color sdlTextureColor = {255, 0, 0, 255};
  SDL_Color sdlTextColor = {0, 0, 0, 255};
  std::shared_ptr<TTF_Font> font =
      Core::AssetStore::Instance().GetFont("pico8-font-10-small");
};

class ImageStreamWindowControls : public IGUISystemWindow {
 public:
  ImageStreamWindowControls(std::optional<Devices::Screen>& screen)
      : screen(screen) {}

  PreviewRectangle windowArea = PreviewRectangle(
      {
          .r = 0x80,
          .g = 0x80,
          .b = 0x80,
      },
      {0x00, 0x00, 0x00, 0xFF}
  );
  PreviewRectangle selectedWindowArea = PreviewRectangle(
      {
          .r = 0x50,
          .g = 0x50,
          .b = 0x50,
      },
      {0x00, 0x00, 0x00, 0xFF}
  );

  std::string GetName() override { return "Window Controls"; }

  void Render(Core::Renderer& renderer) override {
    if (!screen.has_value()) {
      ImGui::Begin(GetName().c_str());
      ImGui::End();
      return;
    }

    ImGui::Begin(GetName().c_str());

    //
    // Draw screen window rectangle according to scale

    auto displaySize = screen->GetSelectedDisplaySize();
    windowArea.UpdateData(displaySize.width, displaySize.height);
    windowArea.RenderSDLTexture(renderer);

    //
    // Draw screen area rectangle according to scale

    selectedWindowArea
        .UpdateData(*screen->width, *screen->height, windowArea.scale);
    selectedWindowArea.RenderSDLTexture(renderer);

    //
    // Render images

    auto windowAreaImGuiOffset = ImGui::GetCursorPos();
    windowArea.RenderImGuiTexture();

    ImGui::SetCursorPos(ImVec2(
        windowAreaImGuiOffset.x + selectedWindowArea.x,
        windowAreaImGuiOffset.y + selectedWindowArea.y
    ));
    ImGui::SetItemAllowOverlap();
    selectedWindowArea.RenderImGuiTexture();

    //
    // Handle drag & drop and resize

    static bool isImageActive = false;
    static bool isResizing = false;

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
      isImageActive = true;
    }

    bool isMouseHoveringItem = ImGui::IsItemHovered();
    bool isMousePastXLimit =
        ImGui::GetMousePos().x >
        selectedWindowArea.x + selectedWindowArea.previewWidth - 10;
    bool isMousePastYLimit =
        ImGui::GetMousePos().y >
        selectedWindowArea.y + selectedWindowArea.previewHeight - 10;
    if (isMouseHoveringItem && isMousePastXLimit && isMousePastYLimit) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
      if (ImGui::IsMouseClicked(0)) {
        isResizing = true;
      }
    } else {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }

    if (isImageActive && !isResizing) {
      if (ImGui::IsMouseDragging(0)) {
        auto tabSize = ImGui::GetContentRegionAvail();
        auto nextX = selectedWindowArea.x + ImGui::GetIO().MouseDelta.x;
        auto nextY = selectedWindowArea.y + ImGui::GetIO().MouseDelta.y;

        if (nextX < 0) {
          nextX = 0;
        }
        if (nextY < 0) {
          nextY = 0;
        }

        if (nextX + selectedWindowArea.previewWidth >
            windowArea.previewWidth + windowArea.x) {
          nextX = windowArea.previewWidth + windowArea.x -
                  selectedWindowArea.previewWidth;
        }
        if (nextY + selectedWindowArea.previewHeight >
            windowArea.previewHeight + windowArea.y) {
          nextY = windowArea.previewHeight + windowArea.y -
                  selectedWindowArea.previewHeight;
        }

        selectedWindowArea.x = nextX;
        selectedWindowArea.y = nextY;

        screen->SetWindowX(selectedWindowArea.x / windowArea.scale);
        screen->SetWindowY(selectedWindowArea.y / windowArea.scale);
      }
    }

    if (isResizing) {
      if (ImGui::IsMouseDragging(0)) {
        selectedWindowArea.previewWidth =
            selectedWindowArea.previewWidth + ImGui::GetIO().MouseDelta.x;
        selectedWindowArea.previewHeight =
            selectedWindowArea.previewHeight + ImGui::GetIO().MouseDelta.y;

        if (selectedWindowArea.previewWidth <= 10) {
          selectedWindowArea.previewWidth = 10;
        }
        if (selectedWindowArea.previewHeight <= 10) {
          selectedWindowArea.previewHeight = 10;
        }

        if (selectedWindowArea.x + selectedWindowArea.previewWidth >
            windowArea.previewWidth) {
          selectedWindowArea.previewWidth =
              windowArea.previewWidth - selectedWindowArea.x;
        }

        if (selectedWindowArea.y + selectedWindowArea.previewHeight >
            windowArea.previewHeight) {
          selectedWindowArea.previewHeight =
              windowArea.previewHeight - selectedWindowArea.y;
        }

        screen->SetSize(
            selectedWindowArea.previewWidth / windowArea.scale,
            selectedWindowArea.previewHeight / windowArea.scale
        );
      }
    }

    if (!ImGui::IsMouseDown(0)) {
      isImageActive = false;
      isResizing = false;
    }

    ImGui::SetCursorPos(ImVec2(
        windowAreaImGuiOffset.x,
        windowAreaImGuiOffset.y + windowArea.previewHeight + 10
    ));

    ImGui::Text("Available screens:");
    for (auto pair : screen->GetDisplaysIndexIdPairs()) {
      auto displayId = std::get<1>(pair);
      std::string buttonLabel = "Screen " + std::to_string(displayId);
      ImGui::SameLine();

      if (ImGui::Button(buttonLabel.c_str())) {
        screen->SetDisplayId(displayId);
      }
    }

    ImGui::Text("Screen position:");
    ImGui::SliderInt(
        "position x",
        &(*screen->windowX),
        0,
        displaySize.width - *screen->width
    );
    ImGui::SliderInt(
        "position y",
        &(*screen->windowY),
        0,
        displaySize.height - *screen->height
    );

    if (*screen->windowX != selectedWindowArea.x / windowArea.scale ||
        *screen->windowY != selectedWindowArea.y / windowArea.scale) {
      selectedWindowArea.x = *screen->windowX * windowArea.scale;
      selectedWindowArea.y = *screen->windowY * windowArea.scale;
    }

    ImGui::Text("Screen size:");
    ImGui::SliderInt(
        "width",
        &(*screen->width),
        10,
        displaySize.width - *screen->windowX
    );
    ImGui::SliderInt(
        "height",
        &(*screen->height),
        10,
        displaySize.height - *screen->windowY
    );

    int currentWidth = *screen->width;
    int currentHeight = *screen->height;
    if (currentWidth != selectedWindowArea.previewWidth / windowArea.scale ||
        currentHeight != selectedWindowArea.previewHeight / windowArea.scale) {
      selectedWindowArea.previewWidth = currentWidth * windowArea.scale;
      selectedWindowArea.previewHeight = currentHeight * windowArea.scale;

      screen->SetSize(currentWidth, currentHeight);
    }

    ImGui::End();
  }

  void Clear() override {
    windowArea.ClearTexture();
    selectedWindowArea.ClearTexture();
  }

 private:
  std::optional<Devices::Screen>& screen;
};
