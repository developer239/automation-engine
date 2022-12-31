#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class SidebarTab2Window : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::LEFT;
  }

  std::string GetName() override { return "Sidebar Tab 2"; }

  void Render(const Devices::Screen& screen, Core::Renderer& renderer)
      override {
    ImGui::Begin(GetName().c_str());
    ImGui::Text("Sidebar tab 2");
    ImGui::End();
  }
};
