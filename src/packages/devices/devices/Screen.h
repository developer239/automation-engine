#pragma once

#include <ApplicationServices/ApplicationServices.h>

#include <opencv2/opencv.hpp>

namespace Devices {

struct DisplaySize {
  int width;
  int height;
};

class Screen {
 public:
  std::shared_ptr<int> width;
  std::shared_ptr<int> height;
  std::shared_ptr<int> windowX;
  std::shared_ptr<int> windowY;
  std::shared_ptr<int> displayId;

  cv::Mat imageOriginal;
  // TODO: rename to imageOutput
  cv::Mat latestScreenshot;

  Screen(int w, int h, int x, int y);

  ~Screen();

  void Screenshot();

  void SetSize(int w, int h);

  std::vector<std::tuple<int, int>> GetDisplaysIndexIdPairs();

  int GetDisplayIndexFromId(int id);

  void SetWindowX(int x) const;

  void SetWindowY(int y) const;

  void SetDisplayId(int id) const;

  [[nodiscard]] DisplaySize GetDisplaySize(int id);

  [[nodiscard]] DisplaySize GetSelectedDisplaySize();

 private:
  CGColorSpaceRef colorSpace;
  CGContextRef contextRef;
  CGImageRef screenshotRef;
};

}  // namespace Devices
