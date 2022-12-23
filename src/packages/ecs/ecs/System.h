#pragma once

#include "Signature.h"

class System {
 private:
  Signature componentSignature;
  std::vector<Entity> entities;

 public:
  System() = default;

  ~System() = default;

  void AddEntityToSystem(Entity entity) { entities.push_back(entity); }

  void RemoveEntityFromSystem(Entity entity) {
    entities.erase(
        std::remove_if(
            entities.begin(),
            entities.end(),
            [&entity](Entity other) { return entity == other; }
        ),
        entities.end()
    );
  }

  std::vector<Entity> GetSystemEntities() const { return entities; }

  const Signature& GetComponentSignature() const { return componentSignature; }

  template <typename TComponent>
  void RequireComponent() {
    const auto componentId = ECSComponent<TComponent>::GetId();
    componentSignature.set(componentId);
  }
};
