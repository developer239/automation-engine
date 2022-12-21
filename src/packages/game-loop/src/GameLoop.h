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
    explicit GameLoop(std::vector<GameLoopStrategy*> strategies);

    ~GameLoop();

    void run();
};
