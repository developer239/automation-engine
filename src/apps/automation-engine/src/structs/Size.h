#pragma once

#include <opencv2/core/types.hpp>

namespace App {

struct Size {
  int width = 0;
  int height = 0;

  Size(int width, int height) : width(width), height(height) {}

  [[nodiscard]] cv::Size ToSize() const { return {width, height}; }
};

}  // namespace App
