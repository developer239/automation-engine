#include "./GameRenderer.h"

GameRenderer::GameRenderer(const std::shared_ptr<SDL_Window>& window) {
  renderer = std::shared_ptr<SDL_Renderer>(
      SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
      SDL_DestroyRenderer
  );
}

void GameRenderer::Render() {
  SDL_RenderPresent(renderer.get());
  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 100, 0);
  SDL_RenderClear(renderer.get());
}
