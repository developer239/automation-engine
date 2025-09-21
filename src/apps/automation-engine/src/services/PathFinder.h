#pragma once

#include <opencv2/core.hpp>
#include <memory>

struct Node {
  cv::Point point;  // the point in the map
  float f = 0;  // g + h
  float g = 0;  // cost from start to current node
  float h = 0;  // heuristic cost from current node to goal
  std::shared_ptr<Node> parent = nullptr;

  Node(cv::Point _point) : point(_point) {}
};

class PathFinder {
 public:
  const int NODE_SIZE = 30;  // Adjust this based on your needs.
  float edgePenaltyScalingFactor = 1.0f;  // Default to 1.0


  std::vector<cv::Point> getOrderedNeighbors(const cv::Point& current, const cv::Point& goal) {
    std::vector<cv::Point> neighbors = {
        {current.x-1, current.y},  // Left
        {current.x+1, current.y},  // Right
        {current.x, current.y-1},  // Up
        {current.x, current.y+1},   // Down
        {current.x-1, current.y-1},  // Top-Left
        {current.x+1, current.y-1},  // Top-Right
        {current.x-1, current.y+1},  // Bottom-Left
        {current.x+1, current.y+1}   // Bottom-Right
    };

    std::sort(neighbors.begin(), neighbors.end(), [&goal](const cv::Point& a, const cv::Point& b) {
      float distA = std::sqrt((a.x - goal.x)*(a.x - goal.x) + (a.y - goal.y)*(a.y - goal.y));
      float distB = std::sqrt((b.x - goal.x)*(b.x - goal.x) + (b.y - goal.y)*(b.y - goal.y));
      return distA < distB;
    });

    return neighbors;
  }

  float computeEdgePenalty(const cv::Point& p, const cv::Mat &walkableMask, float scalingFactor = 1.0f) {
    cv::Point center(walkableMask.cols / 2, walkableMask.rows / 2);
    float distToCenter = std::sqrt((p.x - center.x)*(p.x - center.x) + (p.y - center.y)*(p.y - center.y));
    float maxDist = std::sqrt(center.x*center.x + center.y*center.y);
    return 1.0f + (distToCenter / maxDist) * scalingFactor;
  }


  std::vector<cv::Point> findPathAStar(const cv::Mat &walkableMask, cv::Point start, cv::Point goal) {
    cv::Mat distTransform;
    cv::distanceTransform(walkableMask, distTransform, cv::DIST_L2, 3);
    cv::normalize(distTransform, distTransform, 0.0, 1.0, cv::NORM_MINMAX);

    auto heuristic = [this](cv::Point a, cv::Point b) {
      return std::sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)) * NODE_SIZE;
    };

    // Adjust start and goal points to fit the new resolution.
    start.x /= NODE_SIZE;
    start.y /= NODE_SIZE;
    goal.x /= NODE_SIZE;
    goal.y /= NODE_SIZE;

    std::vector<std::shared_ptr<Node>> openList;
    std::vector<std::shared_ptr<Node>> closedList;
    openList.push_back(std::make_shared<Node>(start));

    while (!openList.empty()) {
      // Find the node with the lowest f in the open list
      auto current = std::min_element(openList.begin(), openList.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
        return a->f < b->f;
      });
      std::shared_ptr<Node> currentNode = *current;

      if (currentNode->point == goal) {
        std::vector<cv::Point> path;
        while (currentNode) {
          path.push_back(cv::Point(currentNode->point.x * NODE_SIZE, currentNode->point.y * NODE_SIZE));
          currentNode = currentNode->parent;
        }
        std::reverse(path.begin(), path.end());
        return path;
      }

      openList.erase(current);
      closedList.push_back(currentNode);

      std::vector<cv::Point> neighbors = getOrderedNeighbors(currentNode->point, goal);

      for (auto& neighborPoint : neighbors) {
        if (neighborPoint.x < 0 || neighborPoint.x * NODE_SIZE >= walkableMask.cols ||
            neighborPoint.y < 0 || neighborPoint.y * NODE_SIZE >= walkableMask.rows ||
            walkableMask.at<uchar>(neighborPoint.y * NODE_SIZE, neighborPoint.x * NODE_SIZE) == 0) {
          continue;
        }

        if (std::find_if(closedList.begin(), closedList.end(), [&neighborPoint](const std::shared_ptr<Node>& a) {
              return a->point == neighborPoint;
            }) != closedList.end()) {
          continue;
        }

        float movementCost = (abs(currentNode->point.x - neighborPoint.x) == 1 &&
                              abs(currentNode->point.y - neighborPoint.y) == 1) ? std::sqrt(2) : 1;

        float penalty = computeEdgePenalty(neighborPoint, walkableMask, edgePenaltyScalingFactor);

        float newG = currentNode->g + movementCost * NODE_SIZE * penalty - distTransform.at<float>(neighborPoint.y * NODE_SIZE, neighborPoint.x * NODE_SIZE);

        std::shared_ptr<Node> neighborNode = std::make_shared<Node>(neighborPoint);
        neighborNode->parent = currentNode;
        neighborNode->g = newG;
        neighborNode->h = heuristic(neighborPoint, goal) - distTransform.at<float>(neighborPoint.y * NODE_SIZE, neighborPoint.x * NODE_SIZE);
        neighborNode->f = neighborNode->g + neighborNode->h;

        auto inOpenList = std::find_if(openList.begin(), openList.end(), [&neighborPoint](const std::shared_ptr<Node>& a) {
          return a->point == neighborPoint;
        });

        if (inOpenList == openList.end() || newG < (*inOpenList)->g) {
          if (inOpenList != openList.end()) {
            openList.erase(inOpenList);
          }
          openList.push_back(neighborNode);
        }
      }
    }

    return {};
  }
};
