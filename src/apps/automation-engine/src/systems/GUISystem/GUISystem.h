#pragma once

#include <curl/curl.h>
#include <opencv2/opencv.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include "../../layout/ImageStreamWindow.h"
#include "./GUISystem.structs.h"
#include "./IGUISystemWindow.h"
#include "./ecs/System.h"

struct GUISystemWindowInfo {
  std::unique_ptr<IGUISystemWindow> window;
  GUISystemLayoutNodePosition position;
};

class GUISystem : public ECS::System {
 public:
  void Render(Core::Renderer& renderer) {
    SetupDock();
    RenderDockSpace();
    RenderWindows(renderer);
  }

  void AfterRender() {
    for (auto& window : windows) {
      window.window->Clear();
    }
  }

  void AddWindow(std::unique_ptr<IGUISystemWindow> window, GUISystemLayoutNodePosition position) {
    windows.push_back({
        .window = std::move(window),
        .position = position
    });
  }

  template <typename TWindow>
  TWindow& GetWindow() {
    for (auto& window : windows) {
      if (auto result = dynamic_cast<TWindow*>(window.window.get())) {
        return *result;
      }
    }
    throw std::runtime_error("Window not found");
  }

 private:
  std::vector<GUISystemWindowInfo> windows;

  ImGuiWindowFlags windowFlags;
  ImGuiDockNodeFlags dockSpaceFlags;

  std::map<GUISystemLayoutNodePosition, ImGuiID> layoutNodes = {
      {GUISystemLayoutNodePosition::LEFT, 0},
      {GUISystemLayoutNodePosition::LEFT_TOP, 0},
      {GUISystemLayoutNodePosition::LEFT_MID, 0},
      {GUISystemLayoutNodePosition::LEFT_BOTTOM, 0},
      {GUISystemLayoutNodePosition::RIGHT, 0},
      {GUISystemLayoutNodePosition::RIGHT_TOP, 0},
      {GUISystemLayoutNodePosition::RIGHT_BOTTOM, 0},
      {GUISystemLayoutNodePosition::RIGHT_BOTTOM_LEFT, 0},
      {GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT, 0},
  };

  void RenderWindows(Core::Renderer& renderer) {
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
      window.window->Render(renderer);
    }
  }

  void BuildNodes(ImGuiID dockSpaceId) {
    auto& leftNode = layoutNodes[GUISystemLayoutNodePosition::LEFT];
    auto& leftTopNode = layoutNodes[GUISystemLayoutNodePosition::LEFT_TOP];
    auto& leftMidNode = layoutNodes[GUISystemLayoutNodePosition::LEFT_MID];
    auto& leftBottomNode = layoutNodes[GUISystemLayoutNodePosition::LEFT_BOTTOM];
    auto& rightNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT];
    auto& rightTopNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT_TOP];
    auto& rightBottomNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT_BOTTOM];
    auto& rightBottomLeftNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT_BOTTOM_LEFT];
    auto& rightBottomRightNode = layoutNodes[GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT];

    //
    // Split base window into two

    ImGui::DockBuilderSplitNode(
        dockSpaceId,
        ImGuiDir_Left,
        0.25f,
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
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(rightBottomNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Split left window into three

    ImGui::DockBuilderSplitNode(leftNode, ImGuiDir_Up, 0.33f, &leftTopNode, &leftMidNode);
    ImGui::DockBuilderSplitNode(leftMidNode, ImGuiDir_Up, 0.25f, &leftMidNode, &leftBottomNode);

    ImGui::DockBuilderGetNode(leftTopNode)->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplitMe |
                                                          ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(leftMidNode)->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplitMe |
                                                          ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(leftBottomNode)->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplitMe |
                                                             ImGuiDockNodeFlags_NoDockingOverMe;

    // Split right bottom window into two
    ImGui::DockBuilderSplitNode(
        ImGui::DockBuilderGetNode(rightBottomNode)->ID,
        ImGuiDir_Left,
        0.66f,
        &rightBottomLeftNode,
        &rightBottomRightNode
    );

    ImGui::DockBuilderGetNode(rightBottomLeftNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(rightBottomRightNode)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Dock windows

    for (auto& window : windows) {
      auto position = window.position;
      ImGui::DockBuilderDockWindow(
          window.window->GetName().c_str(),
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
