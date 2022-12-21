#pragma once

#include <SDL.h>

#include <memory>

class GameWindow {
    std::shared_ptr<SDL_Window> window;

  public:
    GameWindow() {
      SDL_Init(SDL_INIT_EVERYTHING);

      SDL_DisplayMode displayMode;
      SDL_GetCurrentDisplayMode(0, &displayMode);

      window = std::shared_ptr<SDL_Window>(
          SDL_CreateWindow(
              "Automation Engine",
              SDL_WINDOWPOS_CENTERED,
              SDL_WINDOWPOS_CENTERED,
              displayMode.w / 2,
              displayMode.h / 2,
              SDL_WINDOW_RESIZABLE
          ),
          SDL_DestroyWindow
      );
    }

    std::shared_ptr<SDL_Window> get() { return window; }
};
