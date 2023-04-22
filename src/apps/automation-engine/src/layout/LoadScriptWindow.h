#pragma once

#include <sol/sol.hpp>

#include "imgui.h"

#include "../../../externals/ImGuiFileDialog/ImGuiFileDialog.h"
#include "../events/MessageEvent.h"
#include "../events/ScriptFileSelectedEvent.h"
#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class LoadScriptWindow : public IGUISystemWindow {
 public:
  explicit LoadScriptWindow(bool& isRunning) : isRunning(isRunning) {}

  std::vector<MessageEvent> messages = {};

  std::string GetName() override { return "Load Script"; }

  void Render(Core::Renderer& renderer) override {
    ImGui::Begin(GetName().c_str());

    if (filePathName.length() > 0) {
      ImGui::Text("Script loaded");
      ImGui::Spacing();

      if (ImGui::Button("Refresh")) {
        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName
        );
      }

      LiveReload();
    } else {
      ImGui::Text("No script loaded");
    }

    if (ImGui::Button("Open File Dialog")) {
      ImGuiFileDialog::Instance()->OpenDialog(
          "ChooseFileDlgKey",
          "Choose File",
          ".lua",
          // TODO: development value remove in production
          "/Users/michaljarnot/IdeaProjects/flappy-bird-script/dist/"
      );
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName
        );
      }

      ImGuiFileDialog::Instance()->Close();
    }

    if (filePathName.length() > 0) {
      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();
      if (ImGui::Button(isRunning ? "Stop [p]" : "Run [p]")) {
        isRunning = !isRunning;
      }
    }

    ImGui::End();
  }

 private:
  bool& isRunning;
  std::string filePathName;

  // TODO: run in separate thread
  void LiveReload() {
    static Uint32 lastPolledAt = 0;
    auto ticks = SDL_GetTicks();

    if (ticks - lastPolledAt > 1000) {
      lastPolledAt = ticks;

      auto lastWriteTime = std::filesystem::last_write_time(filePathName);
      auto lastWriteTimeMs =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              lastWriteTime.time_since_epoch()
          )
              .count();
      auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now().time_since_epoch()
      )
                             .count();
      auto diff = std::chrono::milliseconds(currentTime - lastWriteTimeMs);
      auto diffS = diff.count() / 1000;

      if (diffS < 1) {
        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName
        );
      }
    }
  }
};
