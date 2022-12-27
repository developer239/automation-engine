#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "./Delay.h"

using ::testing::Return;

TEST(DelayTest, TestDelay) {
  auto start = std::chrono::high_resolution_clock::now();
  Utility::delay(1000);
  auto end = std::chrono::high_resolution_clock::now();

  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  EXPECT_GE(elapsed, 1000);
  EXPECT_LE(elapsed, 1010);
}
