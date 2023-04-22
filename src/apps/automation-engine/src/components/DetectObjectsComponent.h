#pragma once

#include <optional>
#include <utility>

#include "yolo/YOLODetector.h"

#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct DetectObjectsComponent {
  explicit DetectObjectsComponent(
      std::string id = "",
      float confidenceThreshold = 0.3,
      float nonMaximumSuppressionThreshold = 0.3,
      std::string pathToModel = "",
      std::string pathToClasses = ""
  )
      : id(std::move(id)) {
    if (!pathToModel.empty() && !pathToClasses.empty()) {
      net = std::make_shared<YOLO::YOLODetector>(YOLO::NetConfig{
          confidenceThreshold,
          nonMaximumSuppressionThreshold,
          std::move(pathToModel),
          std::move(pathToClasses)});
    }
  }

  std::shared_ptr<YOLO::YOLODetector> net;

  std::string id;
};
