#pragma once

#include <optional>
#include "../structs/Color.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct Operation {
  virtual void Apply(cv::Mat& inputMatrix) = 0;
};

struct MorphologyOperation : public Operation {
  explicit MorphologyOperation(App::Size size = {0, 0}) : size(size) {}

  App::Size size;
};

struct CloseOperation : public MorphologyOperation {
  explicit CloseOperation(App::Size size) : MorphologyOperation(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::morphologyEx(
        inputMatrix,
        inputMatrix,
        cv::MORPH_CLOSE,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

struct DilateOperation : public MorphologyOperation {
  explicit DilateOperation(App::Size size) : MorphologyOperation(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::dilate(
        inputMatrix,
        inputMatrix,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

struct ErodeOperation : public MorphologyOperation {
  explicit ErodeOperation(App::Size size) : MorphologyOperation(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::erode(
        inputMatrix,
        inputMatrix,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

struct OpenOperation : public MorphologyOperation {
  explicit OpenOperation(App::Size size) : MorphologyOperation(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    cv::morphologyEx(
        inputMatrix,
        inputMatrix,
        cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, size.ToSize())
    );
  }
};

struct DetectColorsOperation : public Operation {
  explicit DetectColorsOperation(
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

struct CropOperation : public Operation {
  explicit CropOperation(
      App::Position position = {0, 0}, App::Size size = {0, 0}
  )
      : position(position), size(size) {}

  void Apply(cv::Mat& inputMatrix) override {
    inputMatrix =
        inputMatrix(cv::Rect(position.x, position.y, size.width, size.height));
  }

  App::Position position;
  App::Size size;
};

// TODO implement resize operation (keep in mind that drawing bounding boxes and such (offset) needs to take that into account)

struct DetectionComponent {
  std::vector<std::shared_ptr<Operation>> operations = {};

  void AddArguments(std::shared_ptr<Operation> args) {
    operations.push_back(std::move(args));
  }
};
