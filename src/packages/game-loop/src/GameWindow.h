#pragma once

#include <SDL.h>

#include <memory>

class GameWindow {
    std::shared_ptr<SDL_Window> window;

  public:
    GameWindow();

    std::shared_ptr<SDL_Window> get();
};
