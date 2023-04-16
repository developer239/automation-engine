#pragma once

#include "imgui.h"

#include "../events/MessageEvent.h"
#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class LoggingWindow : public IGUISystemWindow {
 public:
  std::vector<MessageEvent> messages = {};

  void SubscribeToEvents() {
    Events::Bus::Instance().SubscribeToEvent<MessageEvent>(this, &LoggingWindow::OnMessage);
  }

  void OnMessage(MessageEvent& event) { messages.push_back(event); }

  std::string GetName() override { return "Logger"; }

  void Render(Core::Renderer& renderer)
      override {
    DrawImGuiLoggingWindow();
  }

 private:
  bool shouldScrollToBottom = true;
  // TODO: load dynamically from MessageLevel enum
  bool shouldShowInfo = true;
  bool shouldShowWarning = true;
  bool shouldShowError = true;

  void DrawImGuiLoggingWindow() {
    ImGui::Begin(GetName().c_str());

    if (ImGui::Button("Scroll to bottom")) {
      shouldScrollToBottom = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("Clear")) {
      messages.clear();
    }
    ImGui::SameLine();

    // TODO: render dynamically from MessageLevel enum
    ImGui::Checkbox("Info", &shouldShowInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &shouldShowWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &shouldShowError);

    ImGui::Separator();

    ImGui::BeginChild(
        "scrolling",
        ImVec2(0, 0),
        false,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    if (messages.empty()) {
      ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[empty]");
    }

    for (auto& message : messages) {
      if (!shouldShowInfo && message.level == MessageLevel::INFO) {
        continue;
      }
      if (!shouldShowWarning && message.level == MessageLevel::WARNING) {
        continue;
      }
      if (!shouldShowError && message.level == MessageLevel::ERROR) {
        continue;
      }

      if (message.level == MessageLevel::INFO) {
        ImGui::TextColored(
            ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
            message.message.c_str()
        );
      } else if (message.level == MessageLevel::WARNING) {
        ImGui::TextColored(
            ImVec4(0.8f, 0.8f, 0.0f, 1.0f),
            message.message.c_str()
        );
      } else if (message.level == MessageLevel::ERROR) {
        ImGui::TextColored(
            ImVec4(0.8f, 0.0f, 0.0f, 1.0f),
            message.message.c_str()
        );
      }
    }

    if (shouldScrollToBottom) {
      ImGui::SetScrollHereY(1.0f);
    }

    if (ImGui::GetScrollY() < ImGui::GetScrollMaxY()) {
      shouldScrollToBottom = false;
    } else {
      shouldScrollToBottom = true;
    }

    ImGui::EndChild();
    ImGui::End();
  }
};
