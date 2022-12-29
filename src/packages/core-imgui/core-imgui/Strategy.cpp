#include "./Strategy.h"

namespace CoreImGui {

Strategy::~Strategy() {
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void Strategy::Init(Core::Window& window, Core::Renderer& renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL2_InitForSDLRenderer(window.Get().get(), renderer.Get().get());
  ImGui_ImplSDLRenderer_Init(renderer.Get().get());
}

void Strategy::HandleEvent(SDL_Event& event) {
  ImGui_ImplSDL2_ProcessEvent(&event);
}

void Strategy::OnBeforeRender(Core::Window& window, Core::Renderer& renderer) {
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();

  ImGui::NewFrame();
}

void Strategy::OnAfterRender(Core::Window& window, Core::Renderer& renderer) {
  ImGui::Render();

  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

void Strategy::OnRender(Core::Window& window, Core::Renderer& renderer) {}

void Strategy::OnUpdate(Core::Window& window, Core::Renderer& renderer) {}

}  // namespace CoreImGui
