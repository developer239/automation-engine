#include "./Screen.h"

namespace Devices {

Screen::Screen(int w, int h, int x, int y) {
  width = std::make_shared<int>(w);
  height = std::make_shared<int>(h);
  windowX = std::make_shared<int>(x);
  windowY = std::make_shared<int>(y);

  imageOriginal = cv::Mat(cv::Size(*width, *height), CV_8UC4);
  latestScreenshot = cv::Mat(cv::Size(*width, *height), CV_8UC3);

  colorSpace = CGColorSpaceCreateDeviceRGB();
  // TODO: find out why this doesn't work
  //  int targetId = CGMainDisplayID();
  int targetId = 1;
  displayId = std::make_shared<int>(targetId);
}

Screen::~Screen() { CGColorSpaceRelease(colorSpace); }

void Screen::Screenshot() {
  if (imageOriginal.cols != *width || imageOriginal.rows != *height) {
    imageOriginal.create(cv::Size(*width, *height), CV_8UC4);
    latestScreenshot.create(cv::Size(*width, *height), CV_8UC3);
  }

  contextRef = CGBitmapContextCreate(
      imageOriginal.data,
      imageOriginal.cols,
      imageOriginal.rows,
      8,
      imageOriginal.step[0],
      colorSpace,
      kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault
  );

  screenshotRef = CGDisplayCreateImageForRect(
      *displayId,
      CGRectMake(*windowX, *windowY, *width, *height)
  );
  CGContextDrawImage(
      contextRef,
      CGRectMake(0, 0, *width, *height),
      screenshotRef
  );
  cvtColor(imageOriginal, latestScreenshot, cv::COLOR_RGBA2BGR);

  CGImageRelease(screenshotRef);
  CGContextRelease(contextRef);
}

void Screen::SetSize(int w, int h) {
  width.reset(new int(w));
  height.reset(new int(h));

  imageOriginal = cv::Mat(*height, *width, CV_8UC4);
  latestScreenshot = cv::Mat(*height, *width, CV_8UC3);
}

}  // namespace Devices
