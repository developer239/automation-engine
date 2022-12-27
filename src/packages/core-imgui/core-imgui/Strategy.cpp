#include "./Strategy.h"

namespace CoreImGui {

Strategy::~Strategy() {
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void Strategy::Init(Core::Window& window, Core::Renderer& renderer) {
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplSDL2_InitForSDLRenderer(window.Get().get(), renderer.Get().get());
  ImGui_ImplSDLRenderer_Init(renderer.Get().get());
}

void Strategy::HandleEvent(SDL_Event& event) {
  ImGui_ImplSDL2_ProcessEvent(&event);
}

void Strategy::OnRender(Core::Window& window, Core::Renderer& renderer) {
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  ImGui::ShowDemoWindow();
  ImGui::Render();

  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace CoreImGui
