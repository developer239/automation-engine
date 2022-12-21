#pragma once

#include <SDL.h>

#include <memory>

class GameRenderer {
    std::shared_ptr<SDL_Renderer> renderer;

  public:
    explicit GameRenderer(const std::shared_ptr<SDL_Window>& window);

    void Render();

    std::shared_ptr<SDL_Renderer> get() { return renderer; }
};
