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
  std::vector<MessageEvent> messages = {};

  std::string GetName() override { return "Load Script"; }

  void Render(Core::Renderer& renderer) override {
    ImGui::Begin(GetName().c_str());

    if (isFileLoaded) {
      ImGui::Text("Script loaded");
      ImGui::Spacing();
    } else {
      ImGui::Text("No script loaded");
    }

    if (ImGui::Button("Open File Dialog")) {
      ImGuiFileDialog::Instance()->OpenDialog(
          "ChooseFileDlgKey",
          "Choose File",
          ".lua",
          // TODO: development value remove in production
          "/Users/michaljarnot/IdeaProjects/swords-and-souls-scripts/dist/"
      );
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName =
            ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName);
        // Possibly emit and subscribe to event in script system
        isFileLoaded = true;
      }

      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();
  }

  private:
    bool isFileLoaded = false;
};
