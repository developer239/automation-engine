# Automation Engine - Entity Component System

Optional ECS utility package that makes it easy to create and manage entities and components and structure the app
logic.

## Example

```cpp
struct PositionComponent : public ECS::Component<PositionComponent> {
  float x = 0;
  float y = 0;
};

class PositionSystem : public ECS::System {
 public:
  PositionSystem() { RequireComponent<PositionComponent>(); }

  void Update(ECS::Registry& registry) {
    for (auto entity : GetSystemEntities()) {
      auto& component = registry.GetComponent<PositionComponent>(entity);
      component.x += 1;
      component.y += 1;
    }
  }
};

main() {
  ECS::Registry registry;
  ECS::Entity entity;
  PositionSystem positionSystem;
  
  registry.AddComponentToEntity<PositionComponent>(entity);
  registry.AddSystem<PositionSystem>();
    
  auto& position = registry.GetComponent<PositionComponent>(entity);

  EXPECT_FLOAT_EQ(position.x, 0);
  EXPECT_FLOAT_EQ(position.y, 0);

  registry.Update();
  registry.GetSystem<PositionSystem>().Update(registry);

  EXPECT_FLOAT_EQ(position.x, 1);
  EXPECT_FLOAT_EQ(position.y, 1);  
}

```
