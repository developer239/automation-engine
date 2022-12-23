#include "Registry.h"

#include "ECSComponent.h"
#include "Entity.h"
#include "System.h"
#include "gtest/gtest.h"

struct PositionComponent : public ECSComponent<PositionComponent> {
  float x = 0;
  float y = 0;
};

class PositionSystem : public System {
 public:
  PositionSystem() { RequireComponent<PositionComponent>(); }

  void Update(Registry& registry) {
    for (auto entity : GetSystemEntities()) {
      auto& component = registry.GetComponent<PositionComponent>(entity);
      component.x += 1;
      component.y += 1;
    }
  }
};

class ECSIntegrationTest : public testing::Test {
 protected:
  Registry registry;
  Entity entity;
  PositionSystem positionSystem;

  ECSIntegrationTest() : entity(registry.CreateEntity()) {
    registry.AddComponentToEntity<PositionComponent>(entity);
    registry.AddSystem<PositionSystem>();
  }
};

TEST_F(ECSIntegrationTest, TestPositionSystemUpdatesPosition) {
  auto& position =
      registry.GetComponent<PositionComponent>(entity);

  EXPECT_FLOAT_EQ(position.x, 0);
  EXPECT_FLOAT_EQ(position.y, 0);

  registry.Update();
  registry.GetSystem<PositionSystem>().Update(registry);

  EXPECT_FLOAT_EQ(position.x, 1);
  EXPECT_FLOAT_EQ(position.y, 1);
}
