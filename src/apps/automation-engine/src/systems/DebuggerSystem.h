#pragma once

#include "imgui.h"
#include "ecs/System.h"

class DebuggerSystem : public ECS::System {
 public:
  void Render() { ImGui::ShowDemoWindow(); }
};
