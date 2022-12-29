#include "core/Loop.h"

#include "./strategies/ECSStrategy.h"
#include "./strategies/ImguiStrategy.h"

int main() {
  CoreImGui::Strategy gui;
  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
