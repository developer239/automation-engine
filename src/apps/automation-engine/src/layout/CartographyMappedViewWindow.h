#pragma once

#include "imgui.h"

#include "../systems/CartographySystem.h"
#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "../systems/RenderTextSystem.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"
#include "ecs/Registry.h"

class CartographyMappedViewWindow : public IGUISystemWindow {
 public:
  CartographyMappedViewWindow(std::optional<Devices::Screen>& s) : screen(s) {}

  std::string GetName() override { return "Cartography Mapped View Window"; }

  void Render(
      Core::Renderer& renderer
  ) override {
    if(!screen.has_value()) {
      ImGui::Begin(GetName().c_str());
      ImGui::End();
      return;
    }

    ImGui::Begin(GetName().c_str());
    auto cursorTopLeft = ImGui::GetCursorScreenPos();

    auto screenRenderMetadata =
        ECS::Registry::Instance().GetSystem<CartographySystem>().Render(
            renderer
        );
    ImVec2 screenCursor(
        cursorTopLeft.x + screenRenderMetadata.cursor.x,
        cursorTopLeft.y + screenRenderMetadata.cursor.y
    );

    // TODO: remove render text system
    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Render(
        renderer,
        screenCursor,
        screenRenderMetadata.scale
    );

    ImGui::End();
  }

  void Clear() override {
    ECS::Registry::Instance().GetSystem<CartographySystem>().Clear();
    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Clear();
  }

 private:
  std::optional<Devices::Screen>& screen;
};
