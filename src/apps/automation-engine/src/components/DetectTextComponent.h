#pragma once

#include <optional>
#include <utility>

#include "../structs/Color.h"

struct DetectTextComponent {
  explicit DetectTextComponent(
      std::string id = "",
      App::Color bboxColor = {0, 255, 0},
      int bboxThickness = 2,
      bool shouldRenderPreview = false
  )
      : id(std::move(id)),
        bboxColor(bboxColor),
        bboxThickness(bboxThickness),
        shouldRenderPreview(shouldRenderPreview) {}

  std::string id;
  App::Color bboxColor;
  int bboxThickness = 1;
  bool shouldRenderPreview = false;
};
