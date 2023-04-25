#pragma once

#include <opencv2/core/types.hpp>

struct OdometerComponent {
  explicit OdometerComponent(
      bool isRunning = false,
      bool shouldDrawArrow = false,
      bool shouldDebugMatches = false
  )
      : isRunning(isRunning),
        shouldDrawArrow(shouldDrawArrow),
        shouldDebugMatches(shouldDebugMatches) {
    SetDefaultMinimap();
  }

  bool isRunning = false;
  bool shouldDrawArrow = false;
  bool shouldDebugMatches = false;
  std::vector<std::string> ignoreAreas = {"odometer-area-ignore"};

  cv::Mat minimap;
  cv::Point currentPosition;

  void SetDefaultMinimap() {
    int size = 1000;
    minimap = cv::Mat::zeros(size, size, CV_8UC3);
    currentPosition = cv::Point(size/2, size/2);
  }
};
