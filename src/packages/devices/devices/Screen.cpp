#include <thread>

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

  int targetId = CGMainDisplayID();

  // FIXME: possible race condition CGMainDisplayID() returns value that crashes
  // the app during initialization
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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

std::vector<std::tuple<int, int>> Screen::GetDisplaysIndexIdPairs() {
  std::vector<std::tuple<int, int>> result;

  CGDirectDisplayID displayList[16];
  uint32_t displayCount = 0;
  CGGetActiveDisplayList(16, displayList, &displayCount);
  for (int i = 0; i < displayCount; i++) {
    result.emplace_back(i, displayList[i]);
  }

  return result;
}
int Screen::GetDisplayIndexFromId(int id) {
  auto displays = GetDisplaysIndexIdPairs();
  for (auto display : displays) {
    if (std::get<1>(display) == id) {
      return std::get<0>(display);
    }
  }

  return -1;
}

void Screen::SetWindowX(int x) const { *windowX = x; }

void Screen::SetWindowY(int y) const { *windowY = y; }

void Screen::SetDisplayId(int id) const {
  SetWindowY(0);
  SetWindowX(0);
  *displayId = id;
}

}  // namespace Devices
