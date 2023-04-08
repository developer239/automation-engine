#include "./Window.h"
#include <SDL_ttf.h>

namespace Core {

Window::Window() {
  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    throw std::runtime_error("Failed to initialize SDL");
  }
  if (TTF_Init()) {
    throw std::runtime_error("Failed to initialize SDL_ttf");
  }

  SDL_DisplayMode displayMode;
  SDL_GetCurrentDisplayMode(0, &displayMode);

  window = std::shared_ptr<SDL_Window>(
      SDL_CreateWindow(
          "Automation Engine",
          SDL_WINDOWPOS_CENTERED,
          SDL_WINDOWPOS_CENTERED,
          displayMode.w / 4 * 3,
          displayMode.h / 4 * 3,
          SDL_WINDOW_RESIZABLE
      ),
      SDL_DestroyWindow
  );

  if (window == nullptr) {
    throw std::runtime_error("Failed to create SDL window");
  }
}

std::shared_ptr<SDL_Window> Window::Get() { return window; }

}  // namespace Core
