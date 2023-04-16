#pragma once

class IGUISystemWindow {
 public:
  virtual std::string GetName() = 0;

  virtual void Render(Core::Renderer& renderer) = 0;

  virtual void Clear(){};
};
