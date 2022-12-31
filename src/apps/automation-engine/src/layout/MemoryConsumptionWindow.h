#pragma once

#include <mach/mach.h>
#include <malloc/malloc.h>

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class MemoryWindow : public IGUISystemWindow {
 public:
  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::RIGHT_BOTTOM;
  }

  std::string GetName() override { return "Memory"; }

  // memory value
  std::vector<float> values;

  void Render(const Devices::Screen& screen, Core::Renderer& renderer)
      override {
    UpdateMemoryConsumption();
    RenderMemoryGraph();
  }

  void RenderMemoryGraph() {
    float plotWidth = ImGui::GetContentRegionAvail().x;

    // Set the width of the plot
    ImGui::SetNextItemWidth(plotWidth);

    float lastMemoryValue = values.back();
    int minValue = 0;
    int maxValue = 0;
    if (!values.empty()) {
      lastMemoryValue = values[values.size() - 1];
      minValue = *std::min_element(values.begin(), values.end());
      maxValue = *std::max_element(values.begin(), values.end());
    }

    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    auto title =
        "Current memory consumption: " + std::to_string((int)lastMemoryValue) +
        " MB" + "  | min: " + std::to_string(minValue) +
        " MB | max: " + std::to_string(maxValue) + " MB";
    ImGui::PlotLines(
        "",
        values.data(),
        values.size(),
        0,
        title.c_str(),
        0,
        maxValue * 1.4,
        ImVec2(windowSize.x, windowSize.y)
    );
  }

  int lastLoggedAt = 0;

  void UpdateMemoryConsumption() {
    task_vm_info_data_t vm_info;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vm_info, &count);

    auto ticks = SDL_GetTicks();
    if (ticks - lastLoggedAt > 100) {
      auto currentRAMMemoryUsageMB = vm_info.phys_footprint / 1024 / 1024;
      values.push_back(currentRAMMemoryUsageMB);

      if (values.size() > 20) {
        values.erase(values.begin());
      }

      lastLoggedAt = ticks;
    }
  }
};
