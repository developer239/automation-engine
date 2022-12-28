#pragma once

#include <SDL.h>

#include <memory>

#include "./Renderer.h"
#include "./Window.h"

namespace Core {

class IStrategy {
 public:
  virtual void Init(Window& window, Renderer& renderer) = 0;
  virtual void HandleEvent(SDL_Event& event) = 0;
  virtual void OnRender(Window& window, Renderer& renderer) = 0;
  virtual void OnUpdate(Window& window, Renderer& renderer) = 0;
};

}  // namespace Core