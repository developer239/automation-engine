#pragma once

#include "core-imgui/Strategy.h"
#include "core-imgui/IImplementation.h"

class ImguiImplement : public CoreImGui::IImplementation {
  void OnRender() override { ImGui::ShowDemoWindow(); }
};
