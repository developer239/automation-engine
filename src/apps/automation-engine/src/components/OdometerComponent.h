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
        shouldDebugMatches(shouldDebugMatches) {}

  bool isRunning = false;
  bool shouldDrawArrow = false;
  bool shouldDebugMatches = false;
  std::vector<std::string> ignoreAreas = {"odometer-area-ignore"};
};
