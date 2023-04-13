#pragma once

#include <filesystem>
#include <fstream>
#include <sol/sol.hpp>

#include "imgui.h"

#include "../../../externals/ImGuiFileDialog/ImGuiFileDialog.h"
#include "../events/MessageEvent.h"
#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class LoadScriptWindow : public IGUISystemWindow {
 public:
  std::vector<MessageEvent> messages = {};

  LoadScriptWindow(
      std::optional<std::string>& scriptFile, std::optional<sol::state>& lua
  )
      : scriptFile(scriptFile), lua(lua) {
    std::ifstream in(scriptFile->c_str());
    std::string contents(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );
    in.close();

    lua->script(contents);
  }

  std::string GetName() override { return "Load Script"; }

  void Render(Core::Renderer& renderer) override {
    ImGui::Begin(GetName().c_str());

    if (scriptFile.has_value()) {
      auto scriptName = std::filesystem::path(*scriptFile).filename().string();
      ImGui::Text("Current script: %s", scriptName.c_str());
      ImGui::Spacing();
    } else {
      ImGui::Text("No script loaded");
    }

    if (ImGui::Button("Open File Dialog")) {
      ImGuiFileDialog::Instance()
          ->OpenDialog("ChooseFileDlgKey", "Choose File", ".lua", "/Users/michaljarnot/IdeaProjects/swords-and-souls-scripts/dist/");
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        std::string filePathName =
            ImGuiFileDialog::Instance()->GetFilePathName();
        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

        // update scriptFile
      }

      ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();
  }

 private:
  std::optional<std::string>& scriptFile;
  std::optional<sol::state>& lua;
};
