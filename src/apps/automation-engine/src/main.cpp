#include "./strategies/ImguiImplement.h"
#include "./strategies/ECSStrategy.h"
#include "core/Loop.h"

int main() {
  auto guiImplement = ImguiImplement();
  CoreImGui::Strategy gui(&guiImplement);

  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
