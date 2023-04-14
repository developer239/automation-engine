#pragma once

#include <mach/mach.h>
#include <malloc/malloc.h>

#include "imgui.h"

#include "../systems/GUISystem/GUISystem.structs.h"
#include "../systems/GUISystem/IGUISystemWindow.h"
#include "../systems/RenderEditableComponentsGUISystem.h"
#include "./core/Renderer.h"
#include "./devices/Screen.h"

class ManageEntitiesWindow : public IGUISystemWindow {
 public:
  ManageEntitiesWindow(std::optional<Devices::Screen>& screen)
      : screen(screen) {}

  std::string GetName() override { return "Entities"; }

  void Render(Core::Renderer& renderer) override {

    ECS::Registry::Instance()
        .GetSystem<RenderEditableComponentsGUISystem>()
        .Render(screen);

  }

 private:
  std::optional<Devices::Screen>& screen;
};
