#pragma once

#include "core/AssetStore.h"
#include "core/IStrategy.h"
#include "core/Renderer.h"
#include "core/Window.h"
#include "devices/Screen.h"
#include "ecs/Component.h"
#include "ecs/Entity.h"
#include "ecs/Registry.h"
#include "ecs/System.h"
#include "events/Bus.h"

#include "../components/TextLabelComponent.h"
#include "../layout/FPSWindow.h"
#include "../layout/ImageStreamWindow.h"
#include "../layout/ImageStreamWindowControlsWindow.h"
#include "../layout/LoadScriptWindow.h"
#include "../layout/LoggingWindow.h"
#include "../layout/ManageEntitiesWindow.h"
#include "../layout/MemoryWindow.h"
#include "../systems/Detection/DetectContoursSystem.h"
#include "../systems/Detection/DetectTextSystem.h"
#include "../systems/GUISystem/GUISystem.h"
#include "../systems/RenderBoundingBoxSystem.h"
#include "../systems/RenderEditableComponentsGUISystem.h"
#include "../systems/RenderTextSystem.h"
#include "../systems/ScreenSystem.h"
#include "../systems/ScriptingSystem.h"

class ECSStrategy : public Core::IStrategy {
 public:
  void Init(Core::Window& window, Core::Renderer& renderer) override {
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10", "assets/fonts/Roboto-Medium.ttf", 24);
    Core::AssetStore::Instance()
        .AddFont("pico8-font-10-small", "assets/fonts/Roboto-Medium.ttf", 16);

    //
    // Initialize systems

    ECS::Registry::Instance().AddSystem<ScriptingSystem>(screen);
    ECS::Registry::Instance().AddSystem<ScreenSystem>();
    ECS::Registry::Instance().AddSystem<GUISystem>();
    ECS::Registry::Instance().AddSystem<RenderTextSystem>();
    ECS::Registry::Instance().AddSystem<RenderBoundingBoxSystem>();
    ECS::Registry::Instance().AddSystem<RenderEditableComponentsGUISystem>();
    ECS::Registry::Instance().AddSystem<DetectContoursSystem>();
    ECS::Registry::Instance().AddSystem<DetectTextSystem>();

    //
    // Initialize windows

    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindow>(screen),
        GUISystemLayoutNodePosition::RIGHT_TOP
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<LoggingWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_LEFT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<MemoryWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<FPSWindow>(),
        GUISystemLayoutNodePosition::RIGHT_BOTTOM_RIGHT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ImageStreamWindowControls>(screen),
        GUISystemLayoutNodePosition::LEFT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<LoadScriptWindow>(),
        GUISystemLayoutNodePosition::LEFT
    );
    ECS::Registry::Instance().GetSystem<GUISystem>().AddWindow(
        std::make_unique<ManageEntitiesWindow>(screen),
        GUISystemLayoutNodePosition::LEFT
    );
  }

  void HandleEvent(SDL_Event& event) override {}

  void OnUpdate(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance()
        .GetSystem<GUISystem>()
        .GetWindow<LoggingWindow>()
        .SubscribeToEvents();
    ECS::Registry::Instance().GetSystem<ScriptingSystem>().SubscribeToEvents();

    ECS::Registry::Instance().Update();

    if (screen.has_value()) {
      ECS::Registry::Instance().GetSystem<ScreenSystem>().Update(screen);
      ECS::Registry::Instance().GetSystem<ScriptingSystem>().Update();
      ECS::Registry::Instance().GetSystem<DetectContoursSystem>().Update(screen
      );
      ECS::Registry::Instance().GetSystem<DetectTextSystem>().Update(screen);
    }
  }

  void OnRender(Core::Window& window, Core::Renderer& renderer) override {
    if (screen.has_value()) {
      ECS::Registry::Instance().GetSystem<RenderBoundingBoxSystem>().Render(
          screen
      );
    }

    ECS::Registry::Instance().GetSystem<GUISystem>().Render(renderer);
  }

  void OnBeforeRender(Core::Window& window, Core::Renderer& renderer) override {
  }

  void OnAfterRender(Core::Window& window, Core::Renderer& renderer) override {
    ECS::Registry::Instance().GetSystem<GUISystem>().AfterRender();
  }

  ~ECSStrategy() {
    // for some reason std::unique_ptr<tesseract::TessBaseAPI> api; is not
    // cleared unless the system is removed
    ECS::Registry::Instance().RemoveSystem<DetectTextSystem>();
  }

 private:
  std::optional<Devices::Screen> screen;
};
