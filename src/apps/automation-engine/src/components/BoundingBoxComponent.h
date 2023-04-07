#pragma once

#include <opencv2/core/types.hpp>

struct BoundingBoxComponent {
  int positionX;
  int positionY;
  int width;
  int height;
  cv::Scalar color;
  int thickness;

  explicit BoundingBoxComponent(
      const int& positionX = 0,
      const int& positionY = 0,
      const int& width = 0,
      const int& height = 0,
      const cv::Scalar& color = cv::Scalar(255, 0, 0),
      const int& thickness = 1
  ) {
    this->positionX = positionX;
    this->positionY = positionY;
    this->width = width;
    this->height = height;
    this->color = color;
    this->thickness = thickness;
  }
};
