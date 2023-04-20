#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <utility>

#include "../structs/Color.h"
#include "../structs/Position.h"

struct SegmentMaskComponent {
  App::Color color;
  cv::Rect bbox;
  cv::Mat mask;
  bool shouldDrawBbox = true;

  explicit SegmentMaskComponent(
      const App::Color& color = {0, 255, 0},
      const cv::Rect& bbox = {0, 0, 0, 0}, cv::Mat  mask = cv::Mat(),
      bool shouldDrawBbox = true
  )
      : color(color), bbox(bbox), mask(std::move(mask)), shouldDrawBbox(shouldDrawBbox) {}
};