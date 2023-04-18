#pragma once

#include <opencv2/core/types.hpp>

#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct BoundingBoxComponent {
  App::Position position;
  App::Size size;
  App::Color color;
  int thickness;

  explicit BoundingBoxComponent(
      const App::Position& position = {0, 0},
      const App::Size& size = {0, 0},
      const App::Color& color = {0, 255, 0},
      int thickness = 1
  )
      : position(position), size(size), color(color), thickness(thickness) {}
};
