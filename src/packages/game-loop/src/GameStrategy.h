#pragma once

#include <SDL.h>

#include <memory>

#include "./GameRenderer.h"
#include "./GameWindow.h"

class GameLoopStrategy {
 public:
  virtual void Init(GameWindow& window, GameRenderer& renderer) = 0;
  virtual void HandleEvent(SDL_Event& event) = 0;
  virtual void OnRender(GameWindow& window, GameRenderer& renderer) = 0;
};
