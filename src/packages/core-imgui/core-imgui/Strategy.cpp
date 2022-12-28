#include "./Strategy.h"

namespace CoreImGui {

Strategy::~Strategy() {
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void Strategy::Init(Core::Window& window, Core::Renderer& renderer) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplSDL2_InitForSDLRenderer(window.Get().get(), renderer.Get().get());
  ImGui_ImplSDLRenderer_Init(renderer.Get().get());
}

void Strategy::HandleEvent(SDL_Event& event) {
  ImGui_ImplSDL2_ProcessEvent(&event);
}

void Strategy::OnRender(Core::Window& window, Core::Renderer& renderer) {
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();

  ImGui::NewFrame();

  static ImGuiDockNodeFlags dockspace_flags =
      ImGuiDockNodeFlags_PassthruCentralNode;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent
  // window not dockable into, because it would be confusing to have two
  // docking targets within each others.
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will
  // render our background and handle the pass-thru hole, so we ask Begin() to
  // not render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window
  // is collapsed). This is because we want to keep our DockSpace() active. If
  // a DockSpace() is inactive, all active windows docked into it will lose
  // their parent and become undocked. We cannot preserve the docking
  // relationship between an active window and an inactive docking, otherwise
  // any change of dockspace/settings would lead to windows being stuck in
  // limbo and never being visible.
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  // DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    static auto first_time = true;
    if (first_time) {
      first_time = false;

      ImGui::DockBuilderRemoveNode(dockspace_id);  // clear any previous layout
      ImGui::DockBuilderAddNode(
          dockspace_id,
          dockspace_flags | ImGuiDockNodeFlags_DockSpace
      );
      ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

      // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the
      // following args in the following order
      //   window ID to split, direction, fraction (between 0 and 1), the
      //   final two setting let's us choose which id we want (which ever one
      //   we DON'T set as NULL, will be returned by the function)
      //                                                              out_id_at_dir
      //                                                              is the
      //                                                              id of
      //                                                              the node
      //                                                              in the
      //                                                              direction
      //                                                              we
      //                                                              specified
      //                                                              earlier,
      //                                                              out_id_at_opposite_dir
      //                                                              is in
      //                                                              the
      //                                                              opposite
      //                                                              direction
      auto dock_id_left = ImGui::DockBuilderSplitNode(
          dockspace_id,
          ImGuiDir_Left,
          0.2f,
          nullptr,
          &dockspace_id
      );
      auto dock_id_down = ImGui::DockBuilderSplitNode(
          dockspace_id,
          ImGuiDir_Down,
          0.25f,
          nullptr,
          &dockspace_id
      );

      // we now dock our windows into the docking node we made above
      ImGui::DockBuilderDockWindow("Down", dock_id_down);
      ImGui::DockBuilderDockWindow("Left", dock_id_left);

      ImGui::DockBuilderFinish(dockspace_id);
    }
  }

  ImGui::End();

  ImGui::Begin("Left");
  ImGui::Text("Hello, left!");
  ImGui::End();

  ImGui::Begin("Down");
  ImGui::Text("Hello, down!");
  ImGui::End();

  ImGui::Begin("TopRight");
  ImGui::Text("Hello, top right!");
  ImGui::End();

  ImGui::Render();

  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace CoreImGui
