#pragma once

#include <curl/curl.h>
#include <opencv2/opencv.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include "../../layout/ImageStreamWindow.h"
#include "./GUISystem.structs.h"
#include "./IGUISystemWindow.h"
#include "./ecs/System.h"

class GUISystem : public ECS::System {
 public:
  void Render(const Devices::Screen& screen, Core::Renderer& renderer) {
    SetupDock();
    RenderDockSpace();
    RenderWindows(screen, renderer);
  }

  void AddWindow(std::unique_ptr<IGUISystemWindow> window) {
    windows.push_back(std::move(window));
  }

 private:
  std::vector<std::unique_ptr<IGUISystemWindow>> windows;

  ImGuiWindowFlags windowFlags;
  ImGuiDockNodeFlags dockSpaceFlags;

  std::map<GUISystemLayoutNodePosition, ImGuiID> layoutNodes = {
      {GUISystemLayoutNodePosition::LEFT, 0},
      {GUISystemLayoutNodePosition::RIGHT, 0},
      {GUISystemLayoutNodePosition::RIGHT_TOP, 0},
      {GUISystemLayoutNodePosition::RIGHT_BOTTOM, 0},
  };

  void RenderWindows(const Devices::Screen& screen, Core::Renderer& renderer) {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("Menu")) {
        if (ImGui::MenuItem("Exit")) {
          exit(0);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    for (auto& window : windows) {
      ImGui::Begin(window->GetName().c_str());
      window->Render(screen, renderer);
      ImGui::End();
    }
  }

  void BuildNodes(ImGuiID dockSpaceId) {
    auto& leftNode = layoutNodes[GUISystemLayoutNodePosition::LEFT];
    auto& rightNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT];
    auto& rightTopNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT_TOP];
    auto& rightBottomNode =
        layoutNodes[GUISystemLayoutNodePosition::RIGHT_BOTTOM];

    //
    // Split base window into two

    ImGui::DockBuilderSplitNode(
        dockSpaceId,
        ImGuiDir_Left,
        0.33f,
        &leftNode,
        &rightNode
    );

    ImGui::DockBuilderGetNode(leftNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(rightNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Split right window into two

    ImGui::DockBuilderSplitNode(
        ImGui::DockBuilderGetNode(rightNode)->ID,
        ImGuiDir_Up,
        0.66f,
        &rightTopNode,
        &rightBottomNode
    );

    ImGui::DockBuilderGetNode(rightTopNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(rightBottomNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Dock windows

    for (auto& window : windows) {
      auto position = window->GetPosition();
      ImGui::DockBuilderDockWindow(
          window->GetName().c_str(),
          layoutNodes[position]
      );
    }
  }

  void RenderDockSpace() {
    ImGui::Begin("DockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspaceId = ImGui::GetID("Automation Engine");
      ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);

      static auto first_time = true;
      if (first_time) {
        first_time = false;

        // clear any previous layout
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(
            dockspaceId,
            dockSpaceFlags | ImGuiDockNodeFlags_DockSpace
        );
        ImGui::DockBuilderSetNodeSize(
            dockspaceId,
            ImGui::GetMainViewport()->Size
        );

        BuildNodes(dockspaceId);

        ImGui::DockBuilderFinish(dockspaceId);
      }
    }

    ImGui::End();
  }

  void SetupDock() {
    dockSpaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    // Base dock space window shouldn't be dockable
    windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus;

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
    // render our background and handle the pass-thru hole, so we ask Begin() to
    // not render a background.
    if (dockSpaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
      windowFlags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window
    // is collapsed). This is because we want to keep our DockSpace() active. If
    // a DockSpace() is inactive, all active windows docked into it will lose
    // their parent and become undocked. We cannot preserve the docking
    // relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in
    // limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  }
};
