#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class ImageStreamWindowControls : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::LEFT;
  }

  std::string GetName() override { return "Window Controls"; }

  void Render(
      Devices::Screen& screen, Core::Renderer& renderer, Core::Window& window
  ) override {
    ImGui::Begin(GetName().c_str());

    //
    // Calculate window size and scale

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(*screen.displayId - 1, &displayMode);

    ImVec2 windowSize = {(float)displayMode.w, (float)displayMode.h};
    auto scale = CalculateScaleToGuiRegion(windowSize.x, windowSize.y);

    //
    // Draw screen window rectangle according to scale

    static ImVec2 windowRectanglePos = ImVec2(0, 0);
    ImVec2 windowRectangleSize = {
        windowSize.x * scale,
        windowSize.y * scale,
    };

    CreateWindowRectangleTexture(
        renderer,
        windowRectangleSize.x,
        windowRectangleSize.y,
        {
            .r = 0x80,
            .g = 0x80,
            .b = 0x80,
        },
        scale
    );

    //
    // Draw screen area rectangle according to scale

    static ImVec2 screenAreaRectanglePos = ImVec2(0, 0);
    static ImVec2 screenAreaRectangleSize = {
        screen.latestScreenshot.cols * scale,
        screen.latestScreenshot.rows * scale,
    };
    static bool isImageActive = false;
    static bool isResizing = false;

    CreateAreaRectangleTexture(
        renderer,
        screenAreaRectangleSize.x,
        screenAreaRectangleSize.y,
        {
            .r = 0x50,
            .g = 0x50,
            .b = 0x50,
        },
        scale
    );

    //
    // Render images

    ImGui::Image((void*)(intptr_t)textureWindowRectangle, windowRectangleSize);

    ImVec2 fixOffset = {8, 27};  // TODO: find out why this is needed
    ImGui::SetCursorPos(ImVec2(
        screenAreaRectanglePos.x + fixOffset.x,
        screenAreaRectanglePos.y + fixOffset.y
    ));
    ImGui::SetItemAllowOverlap();
    ImGui::Image(
        (void*)(intptr_t)textureAreaRectangle,
        screenAreaRectangleSize
    );

    //
    // Handle drag & drop and resize

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
      isImageActive = true;
    }

    if (ImGui::IsItemHovered() &&
        ImGui::GetMousePos().x >
            screenAreaRectanglePos.x + screenAreaRectangleSize.x - 10 &&
        ImGui::GetMousePos().y >
            screenAreaRectanglePos.y + screenAreaRectangleSize.y - 10) {
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
        auto nextX = screenAreaRectanglePos.x + ImGui::GetIO().MouseDelta.x;
        auto nextY = screenAreaRectanglePos.y + ImGui::GetIO().MouseDelta.y;

        if (nextX < 0) {
          nextX = 0;
        }
        if (nextY < 0) {
          nextY = 0;
        }
        if (nextX + screenAreaRectangleSize.x >
            windowRectangleSize.x + windowRectanglePos.x) {
          nextX = tabSize.x - screenAreaRectangleSize.x;
        }

        // TODO: this is glitching
        //        if (nextY + screenAreaRectangleSize.y >
        //            windowRectangleSize.y + windowRectanglePos.y) {
        //          nextY = tabSize.y - screenAreaRectangleSize.y;
        //        }

        // TODO: glitch workaround
        if (nextY + screenAreaRectangleSize.y >
            windowRectangleSize.y + windowRectanglePos.y) {
          nextY = screenAreaRectanglePos.y;
        }

        screenAreaRectanglePos.x = nextX;
        screenAreaRectanglePos.y = nextY;

        // TODO: use setter
        screen.windowX.reset(new int(screenAreaRectanglePos.x / scale));
        screen.windowY.reset(new int(screenAreaRectanglePos.y / scale));
      }
    }

    if (isResizing) {
      if (ImGui::IsMouseDragging(0)) {
        screenAreaRectangleSize.x =
            screenAreaRectangleSize.x + ImGui::GetIO().MouseDelta.x;
        screenAreaRectangleSize.y =
            screenAreaRectangleSize.y + ImGui::GetIO().MouseDelta.y;

        if (screenAreaRectangleSize.x <= 10) {
          screenAreaRectangleSize.x = 10;
        }
        if (screenAreaRectangleSize.y <= 10) {
          screenAreaRectangleSize.y = 10;
        }

        if (screenAreaRectanglePos.x + screenAreaRectangleSize.x >
            windowRectangleSize.x + windowRectanglePos.x) {
          screenAreaRectangleSize.x =
              windowRectangleSize.x - screenAreaRectanglePos.x;
        }

        if (screenAreaRectanglePos.y + screenAreaRectangleSize.y >
            windowRectangleSize.y + windowRectanglePos.y) {
          screenAreaRectangleSize.y =
              windowRectangleSize.y - screenAreaRectanglePos.y;
        }

        screen.SetSize(
            screenAreaRectangleSize.x / scale,
            screenAreaRectangleSize.y / scale
        );
      }
    }

    if (!ImGui::IsMouseDown(0)) {
      isImageActive = false;
      isResizing = false;
    }

    ImGui::SetCursorPos(ImVec2(
        windowRectanglePos.x + fixOffset.x,
        windowRectanglePos.y + windowRectangleSize.y + fixOffset.y + 10
    ));

    ImGui::Text("Available screens:");

    auto screenCount = GetAvailableScreens();

    for (int i = 0; i < screenCount; i++) {
      std::string buttonLabel = "Screen " + std::to_string(i);
      ImGui::SameLine();

      if (ImGui::Button(buttonLabel.c_str())) {
        screen.displayId.reset(new int(i + 1));

        SDL_DisplayMode displayMode;
        SDL_GetCurrentDisplayMode(i, &displayMode);
      }
    }

    ImGui::Text("Screen position:");
    ImGui::SliderInt(
        "position x",
        &(*screen.windowX),
        0,
        windowSize.x - *screen.width
    );
    ImGui::SliderInt(
        "position y",
        &(*screen.windowY),
        0,
        windowSize.y - *screen.height
    );

    ImGui::Text("Screen size:");
    int currentWidth = *screen.width;
    int currentHeight = *screen.height;
    ImGui::SliderInt(
        "width",
        &currentWidth,
        10,
        windowSize.x - *screen.windowX
    );
    ImGui::SliderInt(
        "height",
        &currentHeight,
        10,
        windowSize.y - *screen.windowY
    );

    if (currentWidth != *screen.width || currentHeight != *screen.height) {
      screen.SetSize(currentWidth, currentHeight);
    }

    ImGui::End();
  }

  void Clear() override {
    SDL_DestroyTexture(textureWindowRectangle);
    SDL_DestroyTexture(textureAreaRectangle);
  }

 private:
  SDL_Texture* textureWindowRectangle{};
  SDL_Texture* textureAreaRectangle{};

  float CalculateScaleToGuiRegion(int width, int height) {
    auto windowSize = ImGui::GetContentRegionAvail();
    float scale = std::min(windowSize.x / width, windowSize.y / height);

    return scale;
  }

  void CreateWindowRectangleTexture(
      Core::Renderer& renderer, int width, int height,
      SDL_Color color = {255, 0, 0, 255}, float scale = 1
  ) {
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0,
        width,
        height,
        32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000
    );
    SDL_FillRect(
        surface,
        nullptr,
        SDL_MapRGB(surface->format, color.r, color.g, color.b)
    );

    // TODO: get from asset manager
    TTF_Font* font = TTF_OpenFont(
        "../../../../src/apps/automation-engine/assets/fonts/Roboto-Medium.ttf",
        14
    );
    if (!font) {
      std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
      return;
    }

    SDL_Color textColor = {0, 0, 0, 255};

    std::string widthString = std::to_string(int(width / scale));
    std::string heightString = std::to_string(int(height / scale));
    std::string label = widthString + "x" + heightString;
    SDL_Surface* textSurface =
        TTF_RenderText_Solid(font, label.c_str(), textColor);
    if (!textSurface) {
      std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
      return;
    }

    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
    SDL_BlitSurface(textSurface, NULL, surface, &textRect);

    // Create the texture from the surface
    textureWindowRectangle =
        SDL_CreateTextureFromSurface(renderer.Get().get(), surface);
    SDL_FreeSurface(surface);

    // Clean up
    TTF_CloseFont(font);
    SDL_FreeSurface(textSurface);
  }

  void CreateAreaRectangleTexture(
      Core::Renderer& renderer, int width, int height,
      SDL_Color color = {255, 0, 0, 255}, float scale = 1
  ) {
    SDL_Surface* surface = SDL_CreateRGBSurface(
        0,
        width,
        height,
        32,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
        0xFF000000
    );
    SDL_FillRect(
        surface,
        nullptr,
        SDL_MapRGB(surface->format, color.r, color.g, color.b)
    );

    // TODO: get from asset manager
    TTF_Font* font = TTF_OpenFont(
        "../../../../src/apps/automation-engine/assets/fonts/Roboto-Medium.ttf",
        14
    );
    if (!font) {
      std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
      return;
    }

    SDL_Color textColor = {0, 0, 0, 255};

    std::string widthString = std::to_string(int(width / scale));
    std::string heightString = std::to_string(int(height / scale));
    std::string label = widthString + "x" + heightString;
    SDL_Surface* textSurface =
        TTF_RenderText_Solid(font, label.c_str(), textColor);
    if (!textSurface) {
      std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
      return;
    }

    SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
    SDL_BlitSurface(textSurface, NULL, surface, &textRect);

    textureAreaRectangle =
        SDL_CreateTextureFromSurface(renderer.Get().get(), surface);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
  }

  int GetAvailableScreens() {
    CGDirectDisplayID displays[16];
    uint32_t displayCount;
    CGGetActiveDisplayList(16, displays, &displayCount);

    //    for (int i = 0; i < displayCount; i++) {
    //      CGDirectDisplayID display = displays[i];
    //      CGRect bounds = CGDisplayBounds(display);
    //      printf(
    //          "Display %d: %f %f %f %f",
    //          i,
    //          bounds.origin.x,
    //          bounds.origin.y,
    //          bounds.size.width,
    //          bounds.size.height
    //      );
    //      std::cout << std::endl;
    //    }

    return displayCount;
  }
};
