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
      Devices::Screen& screen, Core::Renderer& renderer
  ) override {
    ImGui::Begin(GetName().c_str());
    auto cursorTopLeft = ImGui::GetCursorScreenPos();

    auto screenRenderMetadata =
        ECS::Registry::Instance().GetSystem<ScreenSystem>().Render(
            screen,
            renderer
        );
    ImVec2 screenCursor(
        cursorTopLeft.x + screenRenderMetadata.cursor.x,
        cursorTopLeft.y + screenRenderMetadata.cursor.y
    );

    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Render(
        renderer,
        screenCursor,
        screenRenderMetadata.scale
    );

    ImGui::End();
  }

  void Clear() override {
    ECS::Registry::Instance().GetSystem<ScreenSystem>().Clear();
    ECS::Registry::Instance().GetSystem<RenderTextSystem>().Clear();
  }
};
