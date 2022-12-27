#pragma once

#include <SDL.h>

#include <memory>

namespace Core {

class Window {
  std::shared_ptr<SDL_Window> window;

 public:
  Window();

  std::shared_ptr<SDL_Window> Get();
};

}  // namespace Core
