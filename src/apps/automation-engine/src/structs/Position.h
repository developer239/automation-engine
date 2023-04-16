#pragma once

#include <opencv2/core/types.hpp>

namespace App {

struct Position {
  int x = 0;
  int y = 0;

  Position(int x, int y): x(x), y(y) {}

  cv::Vec2i ToVec2i() const { return {x, y}; }
};

}  // namespace App
