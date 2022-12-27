#include "Mouse.h"

#include "gtest/gtest.h"

TEST(MouseTest, MoveTest) {
  auto& mouse = Mouse::getInstance();

  mouse.move(100, 100);

  EXPECT_EQ(mouse.getLocation().x, 100);
  EXPECT_EQ(mouse.getLocation().y, 100);
}
