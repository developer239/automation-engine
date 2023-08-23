#pragma once

#include <vector>
#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class CartographyLocalizerWindow : public IGUISystemWindow {
 public:
  std::string GetName() override { return "Cartography Localizer"; }

  void Render(Core::Renderer& renderer) {
    ImGui::Begin(GetName().c_str());
    ImGui::Text("Cartography Localizer");
    ImGui::End();
  }
};
