#include <SDL.h>

#include <memory>
#include <vector>

#include "externals/imgui/backends/imgui_impl_sdl.h"
#include "externals/imgui/backends/imgui_impl_sdlrenderer.h"
#include "externals/imgui/imgui.h"

class GameWindow {
  std::shared_ptr<SDL_Window> window;

 public:
  GameWindow() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    window = std::shared_ptr<SDL_Window>(
        SDL_CreateWindow(
            "Automation Engine",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            displayMode.w / 2,
            displayMode.h / 2,
            SDL_WINDOW_RESIZABLE
        ),
        SDL_DestroyWindow
    );
  }

  std::shared_ptr<SDL_Window> get() { return window; }
};

class GameRenderer {
  std::shared_ptr<SDL_Renderer> renderer;

 public:
  explicit GameRenderer(const std::shared_ptr<SDL_Window>& window) {
    renderer = std::shared_ptr<SDL_Renderer>(
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED),
        SDL_DestroyRenderer
    );
  }

  void Render() {
    SDL_RenderPresent(renderer.get());
    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 100, 0);
    SDL_RenderClear(renderer.get());
  }

  std::shared_ptr<SDL_Renderer> get() { return renderer; }
};

class GameLoopStrategy {
 public:
  virtual void Init(GameWindow& window, GameRenderer& renderer) = 0;
  virtual void HandleEvent(SDL_Event& event) = 0;
  virtual void OnRender(GameWindow& window, GameRenderer& renderer) = 0;
};

class GUIStrategy : public GameLoopStrategy {
 public:
  ~GUIStrategy() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  void Init(GameWindow& window, GameRenderer& renderer) override {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplSDL2_InitForSDLRenderer(window.get().get(), renderer.get().get());
    ImGui_ImplSDLRenderer_Init(renderer.get().get());
  }

  void HandleEvent(SDL_Event& event) override {
    ImGui_ImplSDL2_ProcessEvent(&event);
  }

  void OnRender(GameWindow& window, GameRenderer& renderer) override {
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
  }
};

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

int main() {
  GUIStrategy gui;
  GameLoop gameLoop({&gui});
  gameLoop.run();
}
