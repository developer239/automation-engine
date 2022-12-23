#pragma once

#include "IComponent.h"

template <typename TComponent>
class ECSComponent : public IComponent {
 public:
  static int GetId() {
    static auto id = nextId++;
    return id;
  }
};
