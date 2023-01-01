#pragma once

class IGUISystemWindow {
 public:
  virtual std::string GetName() = 0;

  virtual GUISystemLayoutNodePosition GetPosition() = 0;

  virtual void Render(Devices::Screen& screen, Core::Renderer& renderer, Core::Window& window) = 0;

  virtual void Clear(){};
};
