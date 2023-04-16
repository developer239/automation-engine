#pragma once

#include <SDL.h>
#include <opencv2/core/types.hpp>

namespace App {

struct Color {
  double r = 0;
  double g = 0;
  double b = 0;

  Color(int r, int g, int b) : r(r), g(g), b(b) {}

  cv::Scalar ToScalar() const { return {b, g, r}; }

  SDL_Color ToSDLColor() const {
    return {
        static_cast<Uint8>(r),
        static_cast<Uint8>(g),
        static_cast<Uint8>(b)};
  }
};

}  // namespace App
