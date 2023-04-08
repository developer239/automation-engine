#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "../systems/RenderTextSystem.h"
#include "../systems/ScreenSystem.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class ImageStreamWindow : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::RIGHT_TOP;
  }

  std::string GetName() override { return "Image Stream"; }

  void Render(
      Devices::Screen& screen, Core::Renderer& renderer, Core::Window& window
  ) override {
    ImGui::Begin(GetName().c_str());
    auto cursorTopLeft = ImGui::GetCursorScreenPos();

    auto screenRenderMetadata =
        ECS::Registry::Instance().GetSystem<ScreenSystem>().Render(
            screen,
            renderer
        );

    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Render(
        renderer,
        {cursorTopLeft.x + screenRenderMetadata.cursor.x,
         cursorTopLeft.y + screenRenderMetadata.cursor.y},
        screenRenderMetadata.scale
    );

    ImGui::End();
  }

  void Clear() override {
    ECS::Registry::Instance().GetSystem<ScreenSystem>().Clear();
    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Clear();
  }
};
