#pragma once

#include "./YOLODetector.h"

YOLO::YOLODetector::YOLODetector(const NetConfig& config) {
  this->confidenceThreshold = config.confidenceThreshold;
  this->nonMaximumSuppressionThreshold =
      config.nonMaximumSuppressionThreshold;

  // Enable cuda part 2
  // OrtStatus* status =
  // OrtSessionOptionsAppendExecutionProvider_CUDA(sessionOptions, 0);
  sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
  ortSession =
      Ort::Session(ortEnv, config.pathToModel.c_str(), sessionOptions);

  LoadTypeInfo();
  LoadClasses(config.pathToClasses);
}
