#pragma once

#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "devices/Screen.h"
#include "ecs/System.h"

#include "../components/OdometerComponent.h"

class OdometerSystem : public ECS::System {
 public:
  cv::Mat prevFrame;

  OdometerSystem() {
    RequireComponent<BoundingBoxComponent>();
    RequireComponent<OdometerComponent>();
  }

  void Update(std::optional<Devices::Screen> screen) {
    // TODO: prevent from crashing (it seems to happen mostly when no points of
    // interest are matched and the screens are vastly different)
    try {
      auto entities = GetSystemEntities();
      if (entities.empty()) {
        return;
      }

      if (entities.size() > 1) {
        std::cout << "OdometerSystem: Found more than one entity with "
                     "OdometerComponent. Only the first one will be used."
                  << std::endl;
      }

      auto entity = entities[0];
      auto& odometerComponent =
          ECS::Registry::Instance().GetEntityComponent<OdometerComponent>(entity
          );

      if (!odometerComponent.isRunning) {
        odometerComponent.SetDefaultMinimap();
        return;
      }

      // TODO: should be limited by screen->width and screen-height (position is
      // fine)
      auto targetAreaBBox =
          ECS::Registry::Instance().GetEntityComponent<BoundingBoxComponent>(
              entity
          );

      auto currentFrame = screen->latestScreenshot.clone();
      currentFrame = currentFrame(cv::Rect(
          targetAreaBBox.position.x,
          targetAreaBBox.position.y,
          targetAreaBBox.size.width,
          targetAreaBBox.size.height
      ));

      if (prevFrame.empty()) {
        prevFrame = currentFrame;
        return;
      }

      if (currentFrame.cols != prevFrame.cols ||
          currentFrame.rows != prevFrame.rows) {
        std::cout << "OdometerSystem: The current frame and the previous frame "
                     "have different dimensions. Skipping till the next frame."
                  << std::endl;
        prevFrame = currentFrame;
        return;
      }

      auto frame1 = prevFrame;
      cv::cvtColor(prevFrame, frame1, cv::COLOR_BGR2GRAY);
      auto frame2 = currentFrame;
      cv::cvtColor(currentFrame, frame2, cv::COLOR_BGR2GRAY);

      //
      // Extract features
      cv::Ptr<cv::Feature2D> feature_extractor = cv::ORB::create();
      std::vector<cv::KeyPoint> keyPoints1, keyPoints2;
      cv::Mat descriptors1, descriptors2;

      feature_extractor
          ->detectAndCompute(frame1, cv::noArray(), keyPoints1, descriptors1);
      feature_extractor
          ->detectAndCompute(frame2, cv::noArray(), keyPoints2, descriptors2);

      //
      // Match features
      cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(
          cv::DescriptorMatcher::BRUTEFORCE_HAMMING
      );
      std::vector<std::vector<cv::DMatch>> knnMatches;
      matcher->knnMatch(descriptors1, descriptors2, knnMatches, 2);

      // TODO: should be limited by screen->width and screen-height (position is
      // fine)
      // TODO: support n entities (for example detected mobs that are moving and
      // corrupt the result) Ignore known entities (for example player area)
      // that might corrupt the result. We are only interested in the background
      // TODO: or experiment and for example group matches by direction/angle
      // and use the largest group (this should kind of filter out moving
      // particles from the screen)
      auto playerEntity = ECS::Registry::Instance().GetEntityByTag("player");
      auto ignoreAreaBBox =
          ECS::Registry::Instance().GetEntityComponent<BoundingBoxComponent>(
              playerEntity
          );
      cv::Rect roi(
          ignoreAreaBBox.position.x - targetAreaBBox.position.x,
          ignoreAreaBBox.position.y - targetAreaBBox.position.y,
          ignoreAreaBBox.size.width,
          ignoreAreaBBox.size.height
      );

      std::vector<cv::DMatch> goodMatches;
      float ratioThreshold = 0.9f;  // Nearest neighbor matching ratio
      for (const auto& m : knnMatches) {
        const cv::Point2f& pt1 = keyPoints1[m[0].queryIdx].pt;
        const cv::Point2f& pt2 = keyPoints2[m[0].trainIdx].pt;

        if (!roi.contains(pt1) && !roi.contains(pt2)) {
          if (m[0].distance < ratioThreshold * m[1].distance) {
            goodMatches.push_back(m[0]);
          }
        }
      }

      //
      // Find the best geometric transformation using RANSAC
      std::vector<cv::Point2f> points1, points2;
      for (const auto& match : goodMatches) {
        points1.push_back(keyPoints1[match.queryIdx].pt);
        points2.push_back(keyPoints2[match.trainIdx].pt);
      }

      //
      // Debug good matches
      if (odometerComponent.shouldDebugMatches) {
        cv::Mat imgMatches;
        cv::drawMatches(
            frame1,
            keyPoints1,
            frame2,
            keyPoints2,
            goodMatches,
            imgMatches
        );
        cv::rectangle(imgMatches, roi, cv::Scalar(0, 255, 0), 2);
        cv::imshow("Odometer Good Matches", imgMatches);
      }

      //
      // Find the homography transformation
      if (goodMatches.size() < 4) {
        std::cout << "Not enough good matches to compute homography"
                  << std::endl;
        prevFrame = currentFrame;
        return;
      }

      //
      // Find the homography transformation
      double ransacReprojectionThreshold = 3.0;
      cv::Mat inliersMask;
      cv::Mat H = cv::findHomography(
          points1,
          points2,
          cv::RANSAC,
          ransacReprojectionThreshold,
          inliersMask
      );

      //
      // Calculate horizontal and vertical displacement in pixels
      cv::Mat H_decomposed = H(cv::Range(0, 2), cv::Range(0, 3));
      cv::Mat translation = H_decomposed.col(2);

      float horizontal_displacement = translation.at<double>(0);
      float vertical_displacement = translation.at<double>(1);

      //
      // Display arrow in the direction of the displacement

      if (odometerComponent.shouldDrawArrow) {
        cv::Point2f arrowStartPoint(frame1.cols / 2.0f, frame1.rows / 2.0f);
        cv::Point2f arrowEndPoint(
            arrowStartPoint.x + horizontal_displacement,
            arrowStartPoint.y + vertical_displacement
        );

        cv::arrowedLine(
            screen->latestScreenshot,
            arrowEndPoint,
            arrowStartPoint,
            cv::Scalar(255, 255, 0),
            2,
            cv::LINE_8,
            0,
            0.3
        );
      }

      if (std::abs(horizontal_displacement) < 5) {
        horizontal_displacement = 0;
      }
      if (std::abs(horizontal_displacement) > 150) {
        horizontal_displacement = 150;
      }
      if (std::abs(vertical_displacement) > 150) {
        vertical_displacement = 150;
      }
      if (std::abs(vertical_displacement) < 5) {
        vertical_displacement = 0;
      }

      //
      // Update captured
      // Calculate the new position based on the displacement
      cv::Point newPosition(
          odometerComponent.currentPosition.x - (horizontal_displacement),
          odometerComponent.currentPosition.y - (vertical_displacement)
      );

      odometerComponent.path.push_back(newPosition);
      odometerComponent.currentPosition = newPosition;
      odometerComponent.captured.setTo(cv::Scalar(0, 0, 0));

      DrawMinimapAroundCurrentPosition(odometerComponent);

      if(odometerComponent.shouldDrawMinimap) {
        cv::Rect roi(
            0,
            0,
            odometerComponent.captured.cols,
            odometerComponent.captured.rows
        );
        cv::Mat destinationROI = screen->latestScreenshot(roi);
        odometerComponent.captured.copyTo(destinationROI);
      }

      prevFrame = currentFrame;
    } catch (const std::exception& e) {
      std::cout << "OdometerSystem: " << e.what() << std::endl;
    }
  }

  void DrawMinimapAroundCurrentPosition(OdometerComponent& component) {
    auto mat = component.minimap;
    auto currentPosition = component.currentPosition;
    auto path = component.path;

    if (path.size() < 2) {
      return;
    }

    auto minimapCenter = cv::Point(mat.cols / 2, mat.rows / 2);
    auto offsetFromCurrentPosition = cv::Point(
        minimapCenter.x - currentPosition.x,
        minimapCenter.y - currentPosition.y
    );

    for (int i = 1; i < path.size(); ++i) {
      auto point1 = path[i - 1];
      auto point2 = path[i];

      auto point1OnMinimap = cv::Point(
          ((point1.x + offsetFromCurrentPosition.x) * component.scale_factor) +
              (mat.cols / 2) - 10,
          ((point1.y + offsetFromCurrentPosition.y) * component.scale_factor) +
              (mat.rows / 2) - 10
      );
      auto point2OnMinimap = cv::Point(
          ((point2.x + offsetFromCurrentPosition.x) * component.scale_factor) +
              (mat.cols / 2) - 10,
          ((point2.y + offsetFromCurrentPosition.y) * component.scale_factor) +
              (mat.rows / 2) - 10
      );

      cv::line(mat, point1OnMinimap, point2OnMinimap, cv::Scalar(0, 255, 0), 3);
    }

    cv::circle(
        component.minimap,
        {int(((component.currentPosition.x + offsetFromCurrentPosition.x) *
              component.scale_factor) +
             (mat.cols / 2) - 10),
         int(((component.currentPosition.y + offsetFromCurrentPosition.y) *
              component.scale_factor) +
             (mat.rows / 2) - 10)},
        3,
        cv::Scalar(255, 0, 0),
        3
    );
  }
};
