#include "./GameLoop.h"

GameLoop::GameLoop(std::vector<GameLoopStrategy*> strategies)
    : renderer(window.get()), strategies(std::move(strategies)) {
  for (auto strategy : this->strategies) {
    strategy->Init(window, renderer);
  }
}

void GameLoop::run() {
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

GameLoop::~GameLoop() { SDL_Quit(); }
