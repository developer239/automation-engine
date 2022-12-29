#pragma once

#include "imgui.h"
#include "ecs/System.h"

class GUISystem : public ECS::System {
 public:
  void Render() { ImGui::ShowDemoWindow(); }
};
