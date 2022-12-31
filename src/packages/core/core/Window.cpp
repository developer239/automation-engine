#include "./Window.h"

namespace Core {

Window::Window() {
  SDL_Init(SDL_INIT_EVERYTHING);

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
}

std::shared_ptr<SDL_Window> Window::Get() { return window; }

}  // namespace Core