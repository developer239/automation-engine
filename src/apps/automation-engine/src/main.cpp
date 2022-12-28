#include "./strategies/ECSStrategy.h"
#include "./strategies/ImguiStrategy.h"
#include "core/Loop.h"

int main() {
  // TODO: this is useless you probably need to get rid of the imgui strategy
  // and implement custom imgui system or call the strategy from the ecs system
  // (probably preferred option)
  auto guiImplement = ImguiStrategy();
  CoreImGui::Strategy gui(&guiImplement);

  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
