#pragma once

#include "devices/Screen.h"
#include "ecs/System.h"
#include "../services/Map.h"
#include "../structs/Position.h"
#include "../structs/Size.h"

struct ROI {
  App::Size size;
  App::Position location;
};

class CartographySystem : public ECS::System {
 public:
  ROI regionToScan = {
      App::Size(115, 90),
      App::Position(1218, 100),

  };
  cv::Mat captured;
  cv::Mat mapped;
  cv::Point lastLocation;

  explicit CartographySystem(
      std::optional<Devices::Screen>& screen, bool& isRunning
  )
      : screen(screen), isRunning(isRunning){};

  void Update() {
    // TODO: only map if isRunning and isMapping
    if (isRunning) {
      captured = screen->latestScreenshot(cv::Rect(
          regionToScan.location.x,
          regionToScan.location.y,
          regionToScan.size.width,
          regionToScan.size.height
      ));

      if (mapped.empty()) {
        mapped = captured.clone();
        return;
      }

      auto result = stitch(mapped, captured, lastLocation);

      // TODO: use matchLoc to stitch not captured but ROI areaToMap or something like that (so that
      // TODO: we can for example use minimap to figure out position and stitch together center of the screen)
      mapped = result.stitched;
      lastLocation = result.matchLoc;

      // TODO: move to render
      auto mappedAreaView = mapped.clone();
      auto lastKnownMappedAreaLocation = lastLocation;
      cv::rectangle(
          mappedAreaView,
          lastKnownMappedAreaLocation,
          cv::Point(
              lastKnownMappedAreaLocation.x + captured.cols,
              lastKnownMappedAreaLocation.y + captured.rows
          ),
          cv::Scalar(0, 0, 255),
          2,
          8,
          0
      );
      cv::imshow("mapped area", mapped);
    }
  }

 private:
  std::optional<Devices::Screen>& screen;
  bool& isRunning;

  // TODO: move to render
  cv::Mat mappedView;
};
