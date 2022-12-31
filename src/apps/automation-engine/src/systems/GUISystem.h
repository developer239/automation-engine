#pragma once

#include <curl/curl.h>
#include <opencv2/opencv.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include "../layout/ImageStreamWindow.h"
#include "./ecs/System.h"



class GUISystem : public ECS::System {
 public:
  void Render(const Devices::Screen& screen, Core::Renderer& renderer) {
    SetupDock();
    RenderDockSpace();
    RenderWindows(screen, renderer);
  }

 private:
  //
  // Windows

  const char LEFT_WINDOW_NAME[15] = "Sidebar Tab";
  const char LEFT_WINDOW_NAME_2[17] = "Sidebar Tab 2";

  const char TOP_RIGHT_WINDOW_NAME[13] = "Image Stream";
  const char BOTTOM_RIGHT_WINDOW_NAME[10] = "Tool Bar";

  ImageStreamWindow imageStreamWindow;

  // ImGui docking setup
  ImGuiWindowFlags windowFlags;
  ImGuiDockNodeFlags dockSpaceFlags;

  void RenderWindows(const Devices::Screen& screen, Core::Renderer& renderer) {
    ImGui::Begin(LEFT_WINDOW_NAME);
    ImGui::Text("Hello, left!");
    ImGui::End();

    ImGui::Begin(LEFT_WINDOW_NAME_2);
    ImGui::Text("Hello, left 2!");
    ImGui::End();

    ImGui::Begin(TOP_RIGHT_WINDOW_NAME);
    imageStreamWindow.Render(screen, renderer);
    ImGui::End();

    ImGui::Begin(BOTTOM_RIGHT_WINDOW_NAME);
    ImGui::Text("Hello, bottom!");
    ImGui::End();
  }

  void BuildNodes(ImGuiID dockSpaceId) {
    ImGuiID leftDockID = 0;
    ImGuiID rightDockID = 0;
    ImGuiID topRightDockID = 0;
    ImGuiID bottomRightDockID = 0;

    //
    // Split base window into two

    ImGui::DockBuilderSplitNode(
        dockSpaceId,
        ImGuiDir_Left,
        0.33f,
        &leftDockID,
        &rightDockID
    );

    ImGui::DockBuilderGetNode(leftDockID)->LocalFlags |=
        ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(rightDockID)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Split right window into two

    ImGui::DockBuilderSplitNode(
        ImGui::DockBuilderGetNode(rightDockID)->ID,
        ImGuiDir_Up,
        0.66f,
        &topRightDockID,
        &bottomRightDockID
    );

    ImGui::DockBuilderGetNode(topRightDockID)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;
    ImGui::DockBuilderGetNode(bottomRightDockID)->LocalFlags |=
        ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDockingSplitMe |
        ImGuiDockNodeFlags_NoDockingOverMe;

    //
    // Dock windows

    ImGui::DockBuilderDockWindow(LEFT_WINDOW_NAME, leftDockID);
    ImGui::DockBuilderDockWindow(TOP_RIGHT_WINDOW_NAME, topRightDockID);
    ImGui::DockBuilderDockWindow(BOTTOM_RIGHT_WINDOW_NAME, bottomRightDockID);

    ImGui::DockBuilderDockWindow(LEFT_WINDOW_NAME_2, leftDockID);
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
