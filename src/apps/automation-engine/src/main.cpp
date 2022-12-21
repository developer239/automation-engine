#include <iostream>

#include "src/GameLoop.h"

int main() {
  auto loop = GameLoop({});
  loop.run();

  std::cout << "hello" << std::endl;
}
