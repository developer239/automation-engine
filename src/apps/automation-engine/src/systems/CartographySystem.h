#pragma once

#include "../services/ParallelTaskManager.h"
#include "../services/PathFinder.h"
#include "../structs/Position.h"
#include "../structs/Size.h"
#include "ScreenSystem.h"
#include "devices/Mouse.h"
#include "devices/Screen.h"
#include "ecs/System.h"

struct ROI {
  App::Size size;
  App::Position location;
};

struct CroppedData {
  // renamed to scannedCropped
  cv::Mat capturedCropped;
  // TODO: rename to mapSearchRegion
  cv::Mat mappedSearchRegion;
  int offsetX;
  int offsetY;
};

struct TemplateMatchResult {
  cv::Point location;
  double confidence;
};

struct StitchResult {
  cv::Mat stitched;
  cv::Point matchLoc;
};

class CartographySystem : public ECS::System {
 public:
  bool isMapping = false;
  bool isLocalizing = false;

  bool isGeneratingWalkableArea = false;
  bool isShowingWalkableArea = false;
  int walkableRadius = 50;

  // We only want to scan part of the screen area that has distinctive features
  // such as a minimap
  ROI regionToScan = {
      App::Size(1200, 650),
      App::Position(25, 175),

  };

  // we don't want to run template matching on the whole map image, but only on
  // a smaller area around the scannedRegion
  int stitchOuterVisibleOffsetOnMapped = 250;
  // in order for template matching to work we can only use a small part of the
  // scannedRegion
  int stitchInnerOffsetForCrop = 200;

  // cv:Mat that contains image of the scanned region that we want to add to map
  cv::Mat scannedRegion;
  // cv:Mat that contains the stitched image of the scanned regions
  cv::Mat map;

  // cv::Mat that contains mask of walkable areas in the map
  cv::Mat walkableMask;

  // x, y coordinates of the regionToScan in the map image
  cv::Point regionLocation;
  // size of the regionToScan in the map image
  App::Size regionLocationSize = App::Size(0, 0);

  // x, y coordinates of the player in the map image
  cv::Point playerLocation;

  cv::Point pathFindingTarget;

  std::vector<cv::Point> path;
  bool isNavigating;
  int navigationCounter = 0;

  explicit CartographySystem(
      std::optional<Devices::Screen>& screen, bool& isRunning
  )
      : screen(screen), isRunning(isRunning){};

  void findPath() {
    auto finder = new PathFinder();
    path =
        finder->findPathAStar(walkableMask, playerLocation, pathFindingTarget);
  }

  void movePlayerToTarget() {
    if (path.empty()) {
      return;
    }

    // Ensure there's a valid screen available
    if (!screen.has_value() || screen->latestScreenshot.empty()) {
      return;  // exit if the screen or screenshot is not available
    }

    cv::Mat& screenshot = screen->latestScreenshot;

    // Calculate the bounding rectangle for the cropped region
    int x = playerLocation.x - screenshot.cols / 2;
    int y = playerLocation.y - screenshot.rows / 2;
    int width = screenshot.cols;
    int height = screenshot.rows;

    // Ensure the bounding rectangle is within the bounds of the walkableMask
    if (x < 0) {
      width += x;  // reduce the width by the overflow amount
      x = 0;
    }
    if (y < 0) {
      height += y;  // reduce the height by the overflow amount
      y = 0;
    }
    if (x + width > walkableMask.cols) {
      width = walkableMask.cols - x;
    }
    if (y + height > walkableMask.rows) {
      height = walkableMask.rows - y;
    }

    if(path.size() > 5) {
      Devices::Mouse::Instance().Move(path[10].x - x, path[10].y - y);
      Devices::Mouse::Instance().Click(kCGMouseButtonLeft, true);
    } else {
      isNavigating = false;
    }
  }

  void projectPathOntoScreenshot() {
    // Ensure there's a valid screen available
    if (!screen.has_value() || screen->latestScreenshot.empty()) {
      return;  // exit if the screen or screenshot is not available
    }

    cv::Mat& screenshot = screen->latestScreenshot;

    // Calculate the bounding rectangle for the cropped region
    int x = playerLocation.x - screenshot.cols / 2;
    int y = playerLocation.y - screenshot.rows / 2;
    int width = screenshot.cols;
    int height = screenshot.rows;

    // Ensure the bounding rectangle is within the bounds of the walkableMask
    if (x < 0) {
      width += x;  // reduce the width by the overflow amount
      x = 0;
    }
    if (y < 0) {
      height += y;  // reduce the height by the overflow amount
      y = 0;
    }
    if (x + width > walkableMask.cols) {
      width = walkableMask.cols - x;
    }
    if (y + height > walkableMask.rows) {
      height = walkableMask.rows - y;
    }

    // Project the cropped region onto the screenshot
    for (auto pathPart : path) {
      if (pathPart.x >= x && pathPart.x <= x + width && pathPart.y >= y &&
          pathPart.y <= y + height) {
        cv::circle(
            screenshot,
            cv::Point{
                pathPart.x - x,
                pathPart.y - y,
            },
            walkableRadius / 2,
            cv::Scalar(255, 0, 255),
            -1
        );
      }
    }

    // project pathfinding target onto screenshot
    cv::circle(
        screenshot,
        {
            pathFindingTarget.x - x,
            pathFindingTarget.y - y,
        },
        walkableRadius * 2,
        cv::Scalar(0, 0, 255),
        -1
    );
  }

  void projectWalkableAreaOntoScreenshot() {
    // Ensure there's a valid screen available
    if (!screen.has_value() || screen->latestScreenshot.empty()) {
      return;  // exit if the screen or screenshot is not available
    }

    cv::Mat& screenshot = screen->latestScreenshot;

    // Calculate the bounding rectangle for the cropped region
    int x = playerLocation.x - screenshot.cols / 2;
    int y = playerLocation.y - screenshot.rows / 2;
    int width = screenshot.cols;
    int height = screenshot.rows;

    // Ensure the bounding rectangle is within the bounds of the walkableMask
    if (x < 0) {
      width += x;  // reduce the width by the overflow amount
      x = 0;
    }
    if (y < 0) {
      height += y;  // reduce the height by the overflow amount
      y = 0;
    }
    if (x + width > walkableMask.cols) {
      width = walkableMask.cols - x;
    }
    if (y + height > walkableMask.rows) {
      height = walkableMask.rows - y;
    }

    cv::Rect cropRect(x, y, width, height);

    // Crop the region from the walkableMask
    cv::Mat croppedWalkable = walkableMask(cropRect);

    // Project the cropped region onto the screenshot
    for (int j = 0; j < croppedWalkable.rows; j++) {
      for (int i = 0; i < croppedWalkable.cols; i++) {
        if (croppedWalkable.at<uchar>(j, i) ==
            255) {  // check if pixel is walkable
          // You can blend or replace pixel color, here's a simple replace
          // approach:
          screenshot.at<cv::Vec3b>(j, i) =
              cv::Vec3b(0, 255, 0);  // Set to green
        }
      }
    }
  }

  void generateWalkableArea() {
    if (!isGeneratingWalkableArea || map.empty()) {
      return;
    }

    // Define a circle centered on the playerLocation with the given radius
    cv::circle(
        walkableMask,
        playerLocation,
        walkableRadius,
        cv::Scalar(255),
        -1
    );
  }

  void setRegionData(const cv::Point& location, const App::Size& size) {
    regionLocation = location;
    regionLocationSize = size;
    playerLocation =
        cv::Point(location.x + size.width / 2, location.y + size.height / 2);
  }

  void setMap(
      const cv::Mat& newMap, std::optional<cv::Mat> walkableMask = std::nullopt
  ) {
    map = newMap.clone();

    if (walkableMask.has_value()) {
      walkableMask = walkableMask.value().clone();
    } else {
      walkableMask = cv::Mat(map.rows, map.cols, CV_8UC1, cv::Scalar(0));
    }
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
                << (x + width) << ", y + height = " << (y + height)
                << std::endl;
      std::cout << "Maze dimensions: " << maze.cols << " x " << maze.rows
                << std::endl;
      throw std::invalid_argument("Region of interest exceeds image dimensions."
      );
    }

    cv::Rect roi(x, y, width, height);
    return maze(roi);
  }

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

  void performMapping() {
    const cv::Mat& mapped = map;
    const cv::Mat& next = scannedRegion;
    const cv::Point& playerLocation = regionLocation;
    int offset = stitchOuterVisibleOffsetOnMapped;
    int MOVE_BY_CROP = stitchInnerOffsetForCrop;

    // Make "next" matrix to performMapping smaller so that we can do template
    // matching
    auto nextSmaller = cropArea(
        next,
        MOVE_BY_CROP,
        MOVE_BY_CROP,
        next.cols - MOVE_BY_CROP * 2,
        next.rows - MOVE_BY_CROP * 2
    );

    // To prevent false positives and improve performance we don't want to do
    // the template matching on the whole image, but only on a smaller area
    // around the player (or where we believe the player is)
    auto x = std::max(0, playerLocation.x - offset);
    auto y = std::max(0, playerLocation.y - offset);
    cv::Mat areaOfInterest =
        // TODO: the width and height here is wrong !! make it work in a similar
        // how we crop images for localization
        cropArea(
            mapped.clone(),
            x,
            y,
            (mapped.cols - x) / 2,
            (mapped.rows - y) / 2
        );

    // Find where nextSmaller is in areaOfInterest
    auto templateMatchResult = templateMatch(areaOfInterest, nextSmaller);

    std::cout << "Match confidence: " << templateMatchResult.confidence
              << std::endl;
    std::cout << "Match location: " << templateMatchResult.location
              << std::endl;

    cv::Point normalizeMatchLoc = {
        templateMatchResult.location.x + x - MOVE_BY_CROP,
        templateMatchResult.location.y + y - MOVE_BY_CROP};

    std::cout << "Normalized match location: " << normalizeMatchLoc
              << std::endl;

    // Adjust the match location to be within the image boundaries
    cv::Point adjustedMatchLoc = {
        std::max(0, normalizeMatchLoc.x),
        std::max(0, normalizeMatchLoc.y)};

    // Calculate the offset of the stitched image to offset match location
    // adjustment
    cv::Point stitchedOffset = {
        std::max(0, -normalizeMatchLoc.x),
        std::max(0, -normalizeMatchLoc.y)};

    int nextOverflowX =
        std::max(0, normalizeMatchLoc.x + next.cols - mapped.cols);
    int nextOverflowY =
        std::max(0, normalizeMatchLoc.y + next.rows - mapped.rows);
    int stitchedCols = mapped.cols + stitchedOffset.x + nextOverflowX;
    int stitchedRows = mapped.rows + stitchedOffset.y + nextOverflowY;

    std::cout << "Stitched dimensions: " << stitchedCols << " cols "
              << stitchedRows << " rows" << std::endl;

    cv::Mat stitched(stitchedRows, stitchedCols, CV_8UC3, cv::Scalar(255));

    std::cout << "Adjusted match location: " << adjustedMatchLoc << std::endl;
    std::cout << "Stitched offset: " << stitchedOffset << std::endl;
    std::cout << "map dimensions: " << mapped.cols << " cols " << mapped.rows
              << " rows" << std::endl;

    // TODO: copy walkableMask to new bigger matrix at correct offset
    //  cv::Mat blah(stitched.rows, stitched.cols, CV_8UC1, cv::Scalar(0));
    //    walkableMask.copyTo(blah(
    //        cv::Rect(stitchedOffset.x, stitchedOffset.y, walkableMask.cols,
    //        walkableMask.rows)
    //    ));
    //  pass to setMap

    mapped.copyTo(stitched(
        cv::Rect(stitchedOffset.x, stitchedOffset.y, mapped.cols, mapped.rows)
    ));

    next.copyTo(stitched(
        cv::Rect(adjustedMatchLoc.x, adjustedMatchLoc.y, next.cols, next.rows)
    ));

    setMap(stitched);

    setRegionData(
        {
            normalizeMatchLoc.x + stitchedOffset.x,
            normalizeMatchLoc.y + stitchedOffset.y,
        },
        {
            regionToScan.location.x,
            regionToScan.location.y,
        }
    );
  }

  void Update() {
    if (isRunning) {
      scannedRegion = screen->latestScreenshot(cv::Rect(
          regionToScan.location.x,
          regionToScan.location.y,
          regionToScan.size.width,
          regionToScan.size.height
      ));
    }

    if (isNavigating && isRunning) {
      pathingTaskManager.Execute([this]() {
        findPath();
        movePlayerToTarget();
      });
    }

    if (isRunning && isMapping) {
      if (map.empty()) {
        setMap(scannedRegion);
        return;
      }

      performMapping();
    }

    if (isLocalizing) {
      performLocalization();
    }

    if (isGeneratingWalkableArea) {
      generateWalkableArea();
    }

    if (isShowingWalkableArea || isGeneratingWalkableArea) {
      // projectWalkableAreaOntoScreenshot();
      projectPathOntoScreenshot();
    }
  }

  CroppedData computeCroppedRegions(
      const cv::Mat& fullImage, const cv::Mat& scannedRegion,
      const cv::Point& desiredLocation, double capturedCropRatio,
      double mappedAreaMultiplier
  ) {
    // 1. Crop the central area from the scannedRegion image based on the given
    // ratio
    int capturedCropWidth = scannedRegion.cols * capturedCropRatio;
    int capturedCropHeight = scannedRegion.rows * capturedCropRatio;
    int capturedOffsetX = (scannedRegion.cols - capturedCropWidth) / 2;
    int capturedOffsetY = (scannedRegion.rows - capturedCropHeight) / 2;

    cv::Mat capturedCropped = scannedRegion(cv::Rect(
        capturedOffsetX,
        capturedOffsetY,
        capturedCropWidth,
        capturedCropHeight
    ));

    // 2. Determine the region in the full image around the desiredLocation
    int mappedAreaWidth = capturedCropped.cols * mappedAreaMultiplier;
    int mappedAreaHeight = capturedCropped.rows * mappedAreaMultiplier;
    int mappedOffsetX = (mappedAreaWidth - capturedCropped.cols) / 2;
    int mappedOffsetY = (mappedAreaHeight - capturedCropped.rows) / 2;

    // Adjust for the desired location coordinates
    int x = desiredLocation.x - mappedOffsetX;
    int y = desiredLocation.y - mappedOffsetY;

    // Ensure we don't go out of bounds for the full image
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + mappedAreaWidth > fullImage.cols)
      x = fullImage.cols - mappedAreaWidth;
    if (y + mappedAreaHeight > fullImage.rows)
      y = fullImage.rows - mappedAreaHeight;

    cv::Mat mappedSearchRegion =
        fullImage(cv::Rect(x, y, mappedAreaWidth, mappedAreaHeight));

    return {capturedCropped, mappedSearchRegion, x, y};
  }

  void performLocalization(bool useWholeImage = false) {
    if (useWholeImage) {
      auto mappedView = map.clone();
      auto capturedView = scannedRegion.clone();
      auto result = templateMatch(mappedView, capturedView);

      setRegionData(
          result.location,
          App::Size(scannedRegion.cols, scannedRegion.rows)
      );
    } else {
      double capturedCropRatio = 1.0;
      double mappedAreaMultiplier = 2.0;

      auto croppedRegions = computeCroppedRegions(
          map,
          scannedRegion,
          regionLocation,
          capturedCropRatio,
          mappedAreaMultiplier
      );
      auto result = templateMatch(
          croppedRegions.mappedSearchRegion,
          croppedRegions.capturedCropped
      );
      auto normalizedResultLocation = cv::Point(
          result.location.x + croppedRegions.offsetX,
          result.location.y + croppedRegions.offsetY
      );

      setRegionData(
          normalizedResultLocation,
          App::Size(scannedRegion.cols, scannedRegion.rows)
      );
    }
  }

  void Clear() { SDL_DestroyTexture(texture); }

  void setTarget() {
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 relativeMousePos =
        ImVec2(mousePos.x - windowPos.x, mousePos.y - windowPos.y);

    float tabBarHeight =
        ImGui::GetFrameHeight();  // Estimate the tab bar height

    // Check if relativeMousePos is within the bounds of the window, excluding
    // the tab bar
    if (relativeMousePos.x >= 0 && relativeMousePos.x <= windowSize.x &&
        relativeMousePos.y >= tabBarHeight &&
        relativeMousePos.y <= windowSize.y) {
      //      std::cout << "Absolute mouse position: x=" << mousePos.x
      //                << ", y=" << mousePos.y << std::endl;
      //      std::cout << "Relative mouse position to window (below the tab
      //      bar): x="
      //                << relativeMousePos.x << ", y=" << relativeMousePos.y
      //                << std::endl;
    }

    // Calculate mouse position relative to the top-left corner of the scaled
    // map image
    ImVec2 posRelativeToScaledMap =
        ImVec2(relativeMousePos.x - cursor.x, relativeMousePos.y - cursor.y);

    // Check if the cursor is within the scaled map image bounds
    if (posRelativeToScaledMap.x >= 0 &&
        posRelativeToScaledMap.x <= scaledWidth &&
        posRelativeToScaledMap.y >= 0 &&
        posRelativeToScaledMap.y <= scaledHeight) {
      // Calculate mouse position relative to the original map
      ImVec2 posRelativeToMap = ImVec2(
          posRelativeToScaledMap.x / scale,
          posRelativeToScaledMap.y / scale
      );

      //      std::cout << "Mouse position relative to map: x=" <<
      //      posRelativeToMap.x
      //                << ", y=" << posRelativeToMap.y << std::endl;
      //      std::cout << "Player location: x=" << playerLocation.x
      //                << ", y=" << playerLocation.y << std::endl;

      // if mouse down mark position
      if (ImGui::IsMouseDown(ImGuiMouseButton_Left)
          // prevent changing tab from clearing the target
          && relativeMousePos.y > 20) {
        std::cout << "Marking target at: x=" << posRelativeToMap.x
                  << ", y=" << posRelativeToMap.y << std::endl;
        pathFindingTarget = {
            static_cast<int>(posRelativeToMap.x),
            static_cast<int>(posRelativeToMap.y)};
      }
    }
  }

  int scaledHeight;
  int scaledWidth;
  ImVec2 cursor;
  float scale;

  ScreenRenderMetadata Render(Core::Renderer& renderer) {
    setTarget();

    auto mappedView = map.clone();

    if (isLocalizing) {
      // draw rectangle for last location center
      auto markerWidth = 100;
      auto markerHeight = 150;

      auto markerTopLeft = cv::Point(
          playerLocation.x - markerWidth / 2,
          playerLocation.y - markerHeight / 2
      );

      // Draw bounding rectangle around the region.
      cv::rectangle(
          mappedView,
          regionLocation,
          cv::Point(
              regionLocation.x + regionLocationSize.width,
              regionLocation.y + regionLocationSize.height
          ),
          cv::Scalar(0, 255, 0),
          20,
          0
      );

      // Draw rectangle to indicate player's position.
      cv::rectangle(
          mappedView,
          markerTopLeft,
          cv::Point(
              markerTopLeft.x + markerWidth,
              markerTopLeft.y + markerHeight
          ),
          cv::Scalar(0, 255, 0),
          20,
          0
      );
    }

    // draw walkable mask on mappedView
    if (isGeneratingWalkableArea || isShowingWalkableArea) {
      cv::Mat walkableMaskBGR;
      cv::cvtColor(walkableMask, walkableMaskBGR, cv::COLOR_GRAY2BGR);
      mappedView = mappedView + walkableMaskBGR;

      cv::circle(
          mappedView,
          cv::Point(pathFindingTarget.x, pathFindingTarget.y),
          walkableRadius * 2,
          cv::Scalar(0, 0, 255),
          -1
      );

      if (!path.empty()) {
        for (auto& point : path) {
          cv::circle(
              mappedView,
              cv::Point(point.x, point.y),
              walkableRadius / 2,
              cv::Scalar(255, 0, 255),
              -1
          );
        }
      }
    }

    if (isGeneratingWalkableArea) {
      cv::circle(
          mappedView,
          cv::Point(pathFindingTarget.x, pathFindingTarget.y),
          10,
          cv::Scalar(255, 255, 0),
          10,
          -1
      );
    }

    cvMatrixAsSDLTexture(mappedView, renderer);

    ImVec2 windowSize = ImGui::GetWindowSize();

    scale = std::min(
        windowSize.x / mappedView.cols,
        windowSize.y / mappedView.rows
    );
    scaledWidth = mappedView.cols * scale;
    scaledHeight = mappedView.rows * scale;

    ImVec2 imageSize = ImVec2(scaledWidth, scaledHeight);
    cursor = ImVec2(
        (windowSize.x - imageSize.x) / 2,
        (windowSize.y - imageSize.y) / 2
    );
    ImGui::SetCursorPos(cursor);
    ImGui::Image(
        (void*)(intptr_t)texture,
        ImVec2(scaledWidth, scaledHeight - 10)
    );

    return {cursor, scale};
  }

 private:
  std::optional<Devices::Screen>& screen;
  bool& isRunning;
  ParallelTaskManager pathingTaskManager;

  SDL_Texture* texture{};

  // TODO: create generic helper
  void cvMatrixAsSDLTexture(cv::Mat& mapped, Core::Renderer& renderer) {
    texture = SDL_CreateTexture(
        renderer.Get().get(),
        SDL_PIXELFORMAT_BGR24,
        SDL_TEXTUREACCESS_STREAMING,
        mapped.cols,
        mapped.rows
    );
    SDL_UpdateTexture(texture, nullptr, (void*)mapped.data, mapped.step1());
  }
};
