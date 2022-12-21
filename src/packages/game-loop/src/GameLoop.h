#pragma once

#include <SDL.h>

#include <memory>
#include <vector>

#include "./GameRenderer.h"
#include "./GameWindow.h"
#include "./GameStrategy.h"

class GameLoop {
    GameWindow window;
    GameRenderer renderer;

    std::vector<GameLoopStrategy*> strategies;

    bool shouldQuit = false;
    SDL_Event event{};

  public:
    explicit GameLoop(std::vector<GameLoopStrategy*> strategies)
        : renderer(window.get()), strategies(std::move(strategies)) {
      for (auto strategy : this->strategies) {
        strategy->Init(window, renderer);
      }
    }

    ~GameLoop() { SDL_Quit(); }

    void run() {
      while (!shouldQuit) {
        while (SDL_PollEvent(&event)) {
          for (auto& strategy : strategies) {
            strategy->HandleEvent(event);
          }

          if (event.type == SDL_QUIT) {
            shouldQuit = true;
          }
        }

        for (auto& strategy : strategies) {
          strategy->OnRender(window, renderer);
        }
        renderer.Render();
      }
    }
};
