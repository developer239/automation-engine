#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class Toolbar2Window : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::RIGHT_BOTTOM;
  }

  std::string GetName() override { return "Toolbar 2"; }

  void Render(const Devices::Screen& screen, Core::Renderer& renderer)
      override {
    ImGui::Text("Toolbar 2");
  }
};
