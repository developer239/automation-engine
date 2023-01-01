#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <opencv2/opencv.hpp>

namespace Devices {

class Screen {
 public:
  std::shared_ptr<int> width;
  std::shared_ptr<int> height;
  std::shared_ptr<int> windowX;
  std::shared_ptr<int> windowY;

  cv::Mat imageOriginal;
  cv::Mat latestScreenshot;

  Screen(int w, int h, int x, int y);

  ~Screen();

  void Screenshot();

  void SetSize(int w, int h);

 private:
  CGColorSpaceRef colorSpace;
  CGContextRef contextRef;
  CGImageRef screenshotRef;
};

}  // namespace Devices