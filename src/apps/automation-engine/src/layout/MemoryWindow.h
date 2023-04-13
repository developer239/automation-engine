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
  std::string GetName() override { return "Memory"; }

  std::vector<float> values;

  void Render(Core::Renderer& renderer)
      override {
    UpdateMemoryConsumption();
    RenderMemoryGraph();
  }

  void RenderMemoryGraph() {
    bool isMemoryLeak = false;
    int interestCount = 50;
    int step = 10;
    if (values.size() > interestCount) {
      for (int i = 0; i < interestCount; i += step) {
        if (values[values.size() - 1 - i] > values[values.size() - 1 - step - i]) {
          isMemoryLeak = true;
        } else {
          isMemoryLeak = false;
          break;
        }
      }
    }

    if (isMemoryLeak) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
    }
    ImGui::Begin(GetName().c_str());
    if (isMemoryLeak) {
      ImGui::PopStyleColor();
    }

    float plotWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetNextItemWidth(plotWidth);

    float lastMemoryValue = values.empty() ? 0.0f : values.back();
    int minValue = 0;
    int maxValue = 0;
    if (!values.empty()) {
      lastMemoryValue = values[values.size() - 1];
      minValue = *std::min_element(values.begin(), values.end());
      maxValue = *std::max_element(values.begin(), values.end());
    }

    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    auto title = "Current: " + std::to_string((int)lastMemoryValue) + " MB" +
                 " | max: " + std::to_string(maxValue) + " MB";

    if (isMemoryLeak) {
      ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    }

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

    if (isMemoryLeak) {
      ImGui::PopStyleColor();
    }
    ImGui::End();
  }

  int lastLoggedAt = 0;

  void UpdateMemoryConsumption() {
    task_vm_info_data_t vm_info;
    mach_msg_type_number_t count = TASK_VM_INFO_COUNT;
    task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vm_info, &count);

    auto ticks = SDL_GetTicks();
    if (ticks - lastLoggedAt > 500) {
      auto currentRAMMemoryUsageMB = vm_info.phys_footprint / 1024 / 1024;
      values.push_back(currentRAMMemoryUsageMB);

      if (values.size() > 100) {
        values.erase(values.begin());
      }

      lastLoggedAt = ticks;
    }
  }
};
