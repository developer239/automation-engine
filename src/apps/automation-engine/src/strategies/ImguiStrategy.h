#pragma once

#include "core-imgui/IImplementation.h"
#include "core-imgui/Strategy.h"

class ImguiStrategy : public CoreImGui::IImplementation {
  void OnRender() override { ImGui::ShowDemoWindow(); }
};
