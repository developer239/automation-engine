#pragma once

#include <vector>
#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class FPSWindow : public IGUISystemWindow {
 public:
  std::string GetName() override { return "FPS"; }

  void Render(Core::Renderer& renderer)
      override {
    ImGui::Begin(GetName().c_str());
    HandleFPSChange();

    int lastFPSValue = 0;
    int minFPS = 0;
    int maxFPS = 0;
    if (!values.empty()) {
      lastFPSValue = values[values.size() - 1];
      minFPS = *std::min_element(values.begin(), values.end());
      maxFPS = *std::max_element(values.begin(), values.end());
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    auto title = "Current: " + std::to_string(lastFPSValue) +
                 " | min: " + std::to_string(minFPS) +
                 " | max: " + std::to_string(maxFPS);
    ImGui::PlotLines(
        "",
        values.data(),
        values.size(),
        0,
        title.c_str(),
        0.0f,
        100.0f,
        ImVec2(windowSize.x - 15, windowSize.y - 35)
    );
    ImGui::End();
  }

 private:
  std::vector<float> values;
  int lastTicks = 0;
  int lastLoggedAt = 0;

  void AddValue(float value) {
    values.push_back(value);

    if (values.size() > 20) {
      values.erase(values.begin());
    }
  }

  int HandleFPSChange() {
    auto ticks = SDL_GetTicks();
    auto delta = ticks - lastTicks;
    lastTicks = ticks;
    auto fps = 1000 / delta;

    if (ticks - lastLoggedAt > 500 && fps > 1) {
      AddValue(fps);
      lastLoggedAt = ticks;
    }
  }
};
