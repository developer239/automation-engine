#include "./strategies/ECSStrategy.h"
#include "./strategies/ImguiStrategy.h"
#include "core/Loop.h"

int main() {
  auto guiImplement = ImguiStrategy();
  CoreImGui::Strategy gui(&guiImplement);

  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
