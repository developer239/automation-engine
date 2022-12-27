#include "./Mouse.h"

#include "gtest/gtest.h"

TEST(MouseTest, MoveTest) {
  auto& mouse = Devices::Mouse::GetInstance();

  mouse.Move(100, 100);

  EXPECT_EQ(mouse.GetLocation().x, 100);
  EXPECT_EQ(mouse.GetLocation().y, 100);
}
