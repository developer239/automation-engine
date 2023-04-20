#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "devices/Keyboard.h"
#include "ecs/Registry.h"

#include "../components/BoundingBoxComponent.h"

bool CheckCollision(ECS::Entity& entityA, ECS::Entity& entityB) {
  auto& boundingBoxA =
      ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(entityA);
  auto& boundingBoxB =
      ECS::Registry::Instance().GetComponent<BoundingBoxComponent>(entityB);

  bool xOverlap = boundingBoxA.position.x + boundingBoxA.size.width >=
                      boundingBoxB.position.x &&
                  boundingBoxB.position.x + boundingBoxB.size.width >=
                      boundingBoxA.position.x;
  bool yOverlap = boundingBoxA.position.y + boundingBoxA.size.height >=
                      boundingBoxB.position.y &&
                  boundingBoxB.position.y + boundingBoxB.size.height >=
                      boundingBoxA.position.y;

  bool touching = xOverlap && yOverlap &&
                  boundingBoxA.position.x <
                      boundingBoxB.position.x + boundingBoxB.size.width &&
                  boundingBoxB.position.x <
                      boundingBoxA.position.x + boundingBoxA.size.width &&
                  boundingBoxA.position.y <
                      boundingBoxB.position.y + boundingBoxB.size.height &&
                  boundingBoxB.position.y <
                      boundingBoxA.position.y + boundingBoxA.size.height;

  return touching;
}

int lastActionAt = 0;
std::map<std::string, int> actionLastAt = {
    {"top", 0}, {"mid", 0}, {"bottom", 0}, {"back", 0}};

std::vector<std::string> areas = {"top", "mid", "bottom"};

int count = 0;

void play() {
  auto now = SDL_GetTicks();
  std::vector<ECS::Entity> apples =
      ECS::Registry::Instance().GetEntitiesByGroup("apple");
  int applesLength = apples.size();

  if (applesLength > 1 && now - lastActionAt > 50) {
    std::sort(
        apples.begin(),
        apples.end(),
        [](const ECS::Entity& a, const ECS::Entity& b) {
          return ECS::Registry::Instance()
                     .GetComponent<BoundingBoxComponent>(a)
                     .position.x > ECS::Registry::Instance()
                                       .GetComponent<BoundingBoxComponent>(b)
                                       .position.x;
        }
    );

    bool isBroken = false;
    for (int i = 0; i < applesLength; i++) {
      if (!isBroken) {
        ECS::Entity apple = apples[i];

        for (std::string area : areas) {
          bool isCollision = false;

          auto areaTop = ECS::Registry::Instance().GetEntityByTag("top");
          auto midTop = ECS::Registry::Instance().GetEntityByTag("mid");
          auto bottomTop = ECS::Registry::Instance().GetEntityByTag("bottom");

          if (area == "top") {
            isCollision = CheckCollision(apple, areaTop);
          } else if (area == "mid") {
            isCollision = CheckCollision(apple, midTop);
          } else if (area == "bottom") {
            isCollision = CheckCollision(apple, bottomTop);
          }

          if (isCollision) {
            int actionThrottle = 151;

            if (now - actionLastAt[area] > actionThrottle) {
              if (area == "top") {
                Devices::Keyboard::Instance().ArrowUp();
              } else if (area == "mid") {
                Devices::Keyboard::Instance().ArrowRight();
              } else if (area == "bottom") {
                Devices::Keyboard::Instance().ArrowDown();
              }

              actionLastAt[area] = SDL_GetTicks();
              lastActionAt = SDL_GetTicks();
            }
          }
        }
      }
    }
  }
}
