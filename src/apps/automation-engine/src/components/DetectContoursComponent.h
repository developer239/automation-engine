#pragma once

#include <optional>
#include <utility>

#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct DetectContoursComponent {
  explicit DetectContoursComponent(
      std::string id = "", App::Size minArea = {10, 10},
      App::Color bboxColor = {0, 255, 0},
      bool shouldRenderPreview = false,
      int bboxThickness = 2,
      std::optional<App::Size> maxArea = std::nullopt
  )
      : id(std::move(id)),
        minArea(minArea),
        bboxColor(bboxColor),
        maxArea(maxArea),
        bboxThickness(bboxThickness),
        shouldRenderPreview(shouldRenderPreview) {}

  std::string id;
  App::Size minArea;
  App::Color bboxColor;
  int bboxThickness = 1;
  std::optional<App::Size> maxArea;
  bool shouldRenderPreview = false;
};
