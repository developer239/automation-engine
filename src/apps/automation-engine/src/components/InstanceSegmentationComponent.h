#pragma once

#include <optional>
#include <utility>

#include "yolo/YOLOSegmentor.h"

#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct InstanceSegmentationComponent {
  explicit InstanceSegmentationComponent(
      std::string id = "",
      float confidenceThreshold = 0.3,
      float nonMaximumSuppressionThreshold = 0.3,
      std::string pathToModel = "",
      std::string pathToClasses = ""
  )
      : id(std::move(id)) {
    if (!pathToModel.empty() && !pathToClasses.empty()) {
      net = std::make_shared<YOLO::YOLOSegmentor>(YOLO::SegNetConfig{
          confidenceThreshold,
          nonMaximumSuppressionThreshold,
          std::move(pathToModel),
          std::move(pathToClasses)});
    }
  }

  std::shared_ptr<YOLO::YOLOSegmentor> net;
  std::string id;
};
