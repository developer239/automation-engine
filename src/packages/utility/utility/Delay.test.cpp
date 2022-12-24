#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "Delay.h"

using ::testing::Return;

class DelayMock {
  public:
    MOCK_METHOD(void, delay, (unsigned int), ());
};

TEST(DelayTest, TestDelay) {
  DelayMock mock;

  ON_CALL(mock, delay(1000)).WillByDefault(::testing::Invoke([]() {
    delay(1000);
  }));

  auto start = std::chrono::high_resolution_clock::now();
  mock.delay(1000);
  auto end = std::chrono::high_resolution_clock::now();

  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  EXPECT_GE(elapsed, 1000);
  EXPECT_LE(elapsed, 1010);
}
