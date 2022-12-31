#pragma once

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

struct IMessage {
  std::string message;
};

class Toolbar1Window : public IGUISystemWindow {
 public:
  std::vector<IMessage> messages = {
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
      {.message = "Hello, world!"}, {.message = "Hello, world!"},
  };

  GUISystemLayoutNodePosition GetPosition() override {
    return GUISystemLayoutNodePosition::RIGHT_BOTTOM;
  }

  std::string GetName() override { return "Debug"; }

  void Render(const Devices::Screen& screen, Core::Renderer& renderer)
      override {
    DrawImGuiLoggingWindow();
  }

  void DrawImGuiLoggingWindow() {
    ImGui::Begin(GetName().c_str());
    ImGui::Button("Clear");
    ImGui::SameLine();
    ImGui::Button("Copy");
    ImGui::Separator();
    ImGui::BeginChild(
        "scrolling",
        ImVec2(0, 0),
        false,
        ImGuiWindowFlags_HorizontalScrollbar
    );
    for (auto& message : messages) {
      ImGui::TextUnformatted(message.message.c_str());
    }

    // scroll to bottom
    ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
  }
};
