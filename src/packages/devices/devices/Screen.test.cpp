#include "./Screen.h"

#include "gtest/gtest.h"

TEST(ScreenTest, TestScreenshotMethodManually) {
  bool wasManuallyEnabled = true;
  if (!wasManuallyEnabled) {
    GTEST_SKIP();
  }

  int width = 800;
  int height = 600;
  int windowX = 0;
  int windowY = 0;

  Screen screen(&width, &height, &windowX, &windowY);

  screen.Screenshot();

  cv::imshow("Screenshot", *screen.latestScreenshot);

  cv::waitKey(0);
}
