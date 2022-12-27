#include "./MatrixAsTexture.h"

void MatrixAsTexture(SDL_Renderer* renderer, cv::Mat* screenshot) {
  SDL_Texture* texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, screenshot->cols,
      screenshot->rows);
  SDL_UpdateTexture(texture, nullptr, (void*) screenshot->data, screenshot->step1());

  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_DestroyTexture(texture);
}
