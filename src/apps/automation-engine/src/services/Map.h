#pragma once
#include <opencv2/opencv.hpp>
#include <random>

cv::Mat cropArea(const cv::Mat& maze, int x, int y, int width, int height) {
  // Check that x and y are within the image boundaries
  if (x < 0 || y < 0 || x >= maze.cols || y >= maze.rows) {
    std::cout << "Invalid x or y: x = " << x << ", y = " << y << std::endl;
    std::cout << "Maze dimensions: " << maze.cols << " x " << maze.rows
              << std::endl;
    throw std::invalid_argument("Invalid x or y for the given image.");
  }

  // Check that width and height are valid
  if (width <= 0 || height <= 0) {
    std::cout << "Invalid width or height: width = " << width
              << ", height = " << height << std::endl;
    throw std::invalid_argument("Width and height must be greater than 0.");
  }

  // Check that the region defined by (x, y, width, height) fits within the
  // image
  if (x + width > maze.cols || y + height > maze.rows) {
    std::cout << "Region of interest exceeds image dimensions: x + width = "
              << (x + width) << ", y + height = " << (y + height) << std::endl;
    std::cout << "Maze dimensions: " << maze.cols << " x " << maze.rows
              << std::endl;
    throw std::invalid_argument("Region of interest exceeds image dimensions.");
  }

  cv::Rect roi(x, y, width, height);
  return maze(roi);
}

struct TemplateMatchResult {
  cv::Point location;
  double confidence;
};

TemplateMatchResult templateMatch(cv::Mat& image, cv::Mat& target) {
  // Ensure image is 2D or grayscale
  if (image.channels() > 1) {
    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
  }

  // Ensure target is 2D or grayscale
  if (target.channels() > 1) {
    cv::cvtColor(target, target, cv::COLOR_BGR2GRAY);
  }

  // Ensure both image and target have the same type
  if (image.type() != target.type()) {
    target.convertTo(target, image.type());
  }

  int result_cols = image.cols - target.cols + 1;
  int result_rows = image.rows - target.rows + 1;

  cv::Mat result;
  result.create(result_rows, result_cols, CV_32FC1);

  cv::matchTemplate(image, target, result, cv::TM_CCOEFF_NORMED);
  cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

  double minVal;
  double maxVal;
  cv::Point minLoc;
  cv::Point maxLoc;

  minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

  return {.location = maxLoc, .confidence = maxVal};
}

struct StitchResult {
  cv::Mat stitched;
  cv::Point matchLoc;
};

StitchResult stitch(
    const cv::Mat& mapped, const cv::Mat& next, const cv::Point& playerLocation,
    // I don't remember what this does
    int offset,
    // I don't remember why I wrote this comment
    // must be less or equal to MOVE_BY and can't be too high (otherwise the
    // result is gonna be finding itself over and over again) if < than MOVE_BY
    // then it
    int MOVE_BY_CROP
) {
  // Make "next" matrix to stitch smaller so that we can do template matching
  auto nextSmaller = cropArea(
      next,
      MOVE_BY_CROP,
      MOVE_BY_CROP,
      next.cols - MOVE_BY_CROP * 2,
      next.rows - MOVE_BY_CROP * 2
  );

  // To prevent false positives and improve performance we don't want to do the
  // template matching on the whole image, but only on a smaller area around the
  // player (or where we believe the player is)
  auto x = std::max(0, playerLocation.x - offset);
  auto y = std::max(0, playerLocation.y - offset);
  cv::Mat areaOfInterest =
      cropArea(mapped.clone(), x, y, mapped.cols - x, mapped.rows - y);

  // Find where nextSmaller is in areaOfInterest
  auto templateMatchResult = templateMatch(areaOfInterest, nextSmaller);

  cv::Point normalizeMatchLoc = {
      std::max(0, templateMatchResult.location.x + x - MOVE_BY_CROP),
      std::max(0, templateMatchResult.location.y + y - MOVE_BY_CROP)};

  int stitchedCols = std::max(mapped.cols, normalizeMatchLoc.x + next.cols);
  int stitchedRows = std::max(mapped.rows, normalizeMatchLoc.y + next.rows);

  cv::Mat stitched(stitchedRows, stitchedCols, CV_8UC3, cv::Scalar(255));

  mapped.copyTo(stitched(cv::Rect(0, 0, mapped.cols, mapped.rows)));
  next.copyTo(stitched(
      cv::Rect(normalizeMatchLoc.x, normalizeMatchLoc.y, next.cols, next.rows)
  ));

  return {stitched, normalizeMatchLoc};
}