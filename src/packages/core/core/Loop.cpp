#include "./Loop.h"

namespace Core {

Loop::Loop(std::vector<IStrategy*> strategies)
    : renderer(window.Get()), strategies(std::move(strategies)) {
  for (auto strategy : this->strategies) {
    strategy->Init(window, renderer);
  }
}

void Loop::Run() {
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
      strategy->OnBeforeRender(window, renderer);
    }

    for (auto& strategy : strategies) {
      strategy->OnUpdate(window, renderer);
      strategy->OnRender(window, renderer);
    }

    for (auto& strategy : strategies) {
      strategy->OnAfterRender(window, renderer);
    }

    renderer.Render();
  }
}

Loop::~Loop() { SDL_Quit(); }

}  // namespace Core
