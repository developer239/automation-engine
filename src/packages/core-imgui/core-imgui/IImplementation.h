#pragma once

namespace CoreImGui {

class IImplementation {
 public:
  virtual void OnRender() = 0;
};

}  // namespace CoreImGui
