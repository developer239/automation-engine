#pragma once

#include <optional>
#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

class Operation {
 public:
  virtual void Apply(cv::Mat& inputMatrix) = 0;
};

class MorphologyArguments : public Operation {
 public:
  explicit MorphologyArguments(App::Size size = {0, 0}) : size(size) {}

  App::Size size;
};

class CloseArguments : public MorphologyArguments {
 public:
  explicit CloseArguments(App::Size size) : MorphologyArguments(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::morphologyEx(
        inputMatrix,
        inputMatrix,
        cv::MORPH_CLOSE,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

class DilateArguments : public MorphologyArguments {
  explicit DilateArguments(App::Size size) : MorphologyArguments(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::dilate(
        inputMatrix,
        inputMatrix,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

class ErodeArguments : public MorphologyArguments {
  explicit ErodeArguments(App::Size size) : MorphologyArguments(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::erode(
        inputMatrix,
        inputMatrix,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

class OpenArguments : public MorphologyArguments {
  explicit OpenArguments(App::Size size) : MorphologyArguments(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::morphologyEx(
        inputMatrix,
        inputMatrix,
        cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

class DetectColorsArguments : public Operation {
  explicit DetectColorsArguments(
      App::Color lowerBound = {0, 0, 0}, App::Color upperBound = {0, 0, 0}
  )
      : lowerBound(lowerBound), upperBound(upperBound) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::inRange(
        inputMatrix,
        lowerBound.ToScalar(),
        upperBound.ToScalar(),
        inputMatrix
    );
  }

  App::Color lowerBound;
  App::Color upperBound;
};

class CropArguments : public Operation {
 public:
  explicit CropArguments(App::Position position = {0, 0}, App::Size size = {0, 0})
      : position(position), size(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    inputMatrix = inputMatrix(cv::Rect(position.x, position.y, size.width, size.height));
  }

  App::Position position;
  App::Size size;
};

class DetectionComponent {
 public:
  std::vector<std::unique_ptr<Operation>> operations = {};

  void AddArguments(std::unique_ptr<Operation> args) {
    operations.push_back(std::move(args));
  }
};
