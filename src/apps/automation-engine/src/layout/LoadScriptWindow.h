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

    if (filePathName.length() > 0) {
      ImGui::Text("Script loaded");
      ImGui::Spacing();

      if (ImGui::Button("Refresh")) {
        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName);
      }
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
        filePathName =
            ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

        Events::Bus::Instance().EmitEvent<ScriptFileSelectedEvent>(filePathName);
      }

      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();
  }

  private:
   std::string filePathName;
};
