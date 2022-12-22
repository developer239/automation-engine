#include <iostream>

#include "src/GameLoop.h"
#include "src/GUIStrategy.h"

int main() {
  GUIStrategy strategy;
  auto loop = GameLoop({
    &strategy
  });
  loop.run();

  std::cout << "hello" << std::endl;
}
