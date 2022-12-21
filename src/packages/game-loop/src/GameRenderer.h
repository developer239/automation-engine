#pragma once

#include <SDL.h>

#include <memory>

class GameRenderer {
    std::shared_ptr<SDL_Renderer> renderer;

  public:
    explicit GameRenderer(const std::shared_ptr<SDL_Window>& window) {
      renderer = std::shared_ptr<SDL_Renderer>(
          SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
          SDL_DestroyRenderer
      );
    }

    void Render() {
      SDL_RenderPresent(renderer.get());
      SDL_SetRenderDrawColor(renderer.get(), 0, 0, 100, 0);
      SDL_RenderClear(renderer.get());
    }

    std::shared_ptr<SDL_Renderer> get() { return renderer; }
};
