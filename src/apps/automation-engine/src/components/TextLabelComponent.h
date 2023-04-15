#pragma once

#include <SDL.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <utility>

#include "../structs/Color.h"
#include "../structs/Position.h"

struct TextLabelComponent {
  std::string text;
  App::Position position;
  App::Color color;
  std::string fontId;

  explicit TextLabelComponent(
      std::string text = "",
      const App::Position& position = {0, 0},
      const App::Color& color = {0, 255, 0},
      std::string assetId = "pico8-font-10"
  )
      : text(std::move(text)),
        position(position),
        color(color),
        fontId(std::move(assetId)) {}
};
