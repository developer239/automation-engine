#include "./strategies/ImguiImplement.h"
#include "core/Loop.h"

int main() {
  auto implement = ImguiImplement();
  CoreImGui::Strategy gui(&implement);

  auto loop = Core::Loop({&gui});
  loop.Run();
}
