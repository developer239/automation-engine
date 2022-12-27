#include "core/Loop.h"
#include "core-imgui/Strategy.h"

int main() {
  CoreImGui::Strategy gui;

  auto loop = Core::Loop({
    &gui
  });
  loop.Run();
}
