#pragma once

#include "../../../../externals/imgui/backends/imgui_impl_sdl.h"
#include "../../../../externals/imgui/backends/imgui_impl_sdlrenderer.h"
#include "../../../../externals/imgui/imgui.h"
#include "core/IStrategy.h"
#include "core/Renderer.h"
#include "core/Window.h"

namespace CoreImGui {

class Strategy : public Core::IStrategy {
 public:
  ~Strategy();

  void Init(Core::Window& window, Core::Renderer& renderer) override;

  void HandleEvent(SDL_Event& event) override;

  void OnRender(Core::Window& window, Core::Renderer& renderer) override;
};

}  // namespace CoreImGui