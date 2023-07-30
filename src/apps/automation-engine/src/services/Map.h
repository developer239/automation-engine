#pragma once
#include <opencv2/opencv.hpp>
#include <random>

void carve(cv::Mat& maze, int r, int c) {
  static const int dr[] = {-1, 1, 0, 0};
  static const int dc[] = {0, 0, -1, 1};

  std::random_device rd;
  std::mt19937 generator(rd());
  std::vector<int> d{0, 1, 2, 3};
  std::shuffle(d.begin(), d.end(), generator);

  for (int i = 0; i < 4; ++i) {
    int rr = r + 2 * dr[d[i]];
    int cc = c + 2 * dc[d[i]];

    if (rr < 0 || rr >= maze.rows || cc < 0 || cc >= maze.cols ||
        maze.at<uchar>(rr, cc) == 0)
      continue;

    maze.at<uchar>(r + dr[d[i]], c + dc[d[i]]) = 0;
    maze.at<uchar>(rr, cc) = 0;

    carve(maze, rr, cc);
  }
}

cv::Mat generateMaze(int rows, int cols, int pathSize) {
  int actualRows = 2 * (rows / (2 * pathSize)) + 1;
  int actualCols = 2 * (cols / (2 * pathSize)) + 1;

  cv::Mat maze(actualRows, actualCols, CV_8UC1, cv::Scalar(255));

  std::random_device rd;
  std::mt19937 generator(rd());
  int startRow =
      std::uniform_int_distribution<int>{0, actualRows / 2}(generator)*2 + 1;
  int startCol =
      std::uniform_int_distribution<int>{0, actualCols / 2}(generator)*2 + 1;
  maze.at<uchar>(startRow, startCol) = 0;

  carve(maze, startRow, startCol);

  cv::Mat largeMaze;
  cv::resize(maze, largeMaze, cv::Size(cols, rows), 0, 0, cv::INTER_NEAREST);
  return largeMaze;
}

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

cv::Mat cropRandomArea(cv::Mat& maze, int width, int height) {
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<int> xDist{0, maze.cols - width};
  std::uniform_int_distribution<int> yDist{0, maze.rows - height};

  int x = xDist(generator);
  int y = yDist(generator);

  return cropArea(maze, x, y, width, height);
}

cv::Point templateMatch(cv::Mat& image, cv::Mat& target) {
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
  cv::Point matchLoc;

  minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

  matchLoc = maxLoc;

  return matchLoc;
}

// must be less or equal to MOVE_BY and can't be too high (otherwise the result
// is gonna be finding itself over and over again)
// if < than MOVE_BY then it
int MOVE_BY_CROP = 300;

struct StitchResult {
  cv::Mat stitched;
  cv::Point matchLoc;
};

StitchResult stitch(
    const cv::Mat& mapped, const cv::Mat& next, const cv::Point& playerLocation,
    int offset = 100
) {
  // Convert to 3 channels if necessary
  cv::Mat nextConverted, mappedConverted;

  if (mapped.channels() == 1)
    cvtColor(mapped, mappedConverted, cv::COLOR_GRAY2BGR);
  else
    mappedConverted = mapped;

  if (next.channels() == 1)
    cvtColor(next, nextConverted, cv::COLOR_GRAY2BGR);
  else
    nextConverted = next;

  auto nextSmaller = cropArea(
      nextConverted,
      MOVE_BY_CROP,
      MOVE_BY_CROP,
      nextConverted.cols - MOVE_BY_CROP * 2,
      nextConverted.rows - MOVE_BY_CROP * 2
  );

  auto x = std::max(0, playerLocation.x - offset);
  auto y = std::max(0, playerLocation.y - offset);

  cv::Mat areaOfInterest =
      cropArea(mappedConverted.clone(), x, y, mappedConverted.cols - x, mappedConverted.rows - y);

  cv::Point matchLoc = templateMatch(areaOfInterest, nextSmaller);

  cv::Point normalizeMatchLoc = {
      std::max(0, matchLoc.x + x - MOVE_BY_CROP),
      std::max(0, matchLoc.y + y - MOVE_BY_CROP)};

  int stitchedCols = std::max(mappedConverted.cols, normalizeMatchLoc.x + nextConverted.cols);
  int stitchedRows = std::max(mappedConverted.rows, normalizeMatchLoc.y + nextConverted.rows);

  cv::Mat stitched(stitchedRows, stitchedCols, CV_8UC3, cv::Scalar(255));

  mappedConverted.copyTo(stitched(cv::Rect(0, 0, mappedConverted.cols, mappedConverted.rows)));
  nextConverted.copyTo(stitched(cv::Rect(normalizeMatchLoc.x, normalizeMatchLoc.y, nextConverted.cols, nextConverted.rows)));

  return {stitched, normalizeMatchLoc};
}
