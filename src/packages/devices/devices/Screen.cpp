#include "./Screen.h"

namespace Devices {

Screen::Screen(int* w, int* h, int* x, int* y) {
  width = w;
  height = h;
  windowX = x;
  windowY = y;

  imageOriginal = std::make_unique<cv::Mat>(cv::Size(*width, *height), CV_8UC4);
  latestScreenshot = new cv::Mat(cv::Size(*width, *height), CV_8UC3);

  colorSpace = CGColorSpaceCreateDeviceRGB();
}

Screen::~Screen() {
  CGColorSpaceRelease(colorSpace);

  delete latestScreenshot;
}

void Screen::Screenshot() {
  if (imageOriginal->cols != *width || imageOriginal->rows != *height) {
    imageOriginal->create(cv::Size(*width, *height), CV_8UC4);
    latestScreenshot->create(cv::Size(*width, *height), CV_8UC3);
  }

  contextRef = CGBitmapContextCreate(
      imageOriginal->data,
      imageOriginal->cols,
      imageOriginal->rows,
      8,
      imageOriginal->step[0],
      colorSpace,
      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault
  );

  screenshotRef = CGDisplayCreateImageForRect(
      CGMainDisplayID(),
      CGRectMake(*windowX, *windowY, *width, *height)
  );
  CGContextDrawImage(
      contextRef,
      CGRectMake(0, 0, *width, *height),
      screenshotRef
  );
  cvtColor(*imageOriginal, *latestScreenshot, cv::COLOR_RGBA2BGR);

  CGImageRelease(screenshotRef);
  CGContextRelease(contextRef);
}

}  // namespace Devices