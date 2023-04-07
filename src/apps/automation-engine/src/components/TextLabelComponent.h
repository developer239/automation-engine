#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <SDL.h>

struct TextLabelComponent {
  cv::Vec2i position;
  std::string text;
  SDL_Color color;
  std::string fontId;

  explicit TextLabelComponent(
      const cv::Vec2i& position = cv::Vec2i(0),
      const std::string& text = "",
      const SDL_Color& color = { 0, 255, 0 },
      const std::string& assetId = "pico8-font-10"
  ) {
    this->position = position;
    this->text = text;
    this->fontId = assetId;
    this->color = color;
  }
};
