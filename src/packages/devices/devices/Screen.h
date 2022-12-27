#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <opencv2/opencv.hpp>

class Screen {
 public:
  int* width;
  int* height;
  int* windowX;
  int* windowY;

  std::unique_ptr<cv::Mat> imageOriginal;
  cv::Mat* latestScreenshot;

  Screen(int* w, int* h, int* x, int* y);

  ~Screen();

  void Screenshot();

 private:
  CGColorSpaceRef colorSpace;
  CGContextRef contextRef;
  CGImageRef screenshotRef;
};
