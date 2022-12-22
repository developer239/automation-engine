#pragma once

#include "src/GameStrategy.h"
#include "src/GameWindow.h"
#include "src/GameRenderer.h"

#include "../../../../externals/imgui/backends/imgui_impl_sdlrenderer.h"
#include "../../../../externals/imgui/backends/imgui_impl_sdl.h"
#include "../../../../externals/imgui/imgui.h"

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
