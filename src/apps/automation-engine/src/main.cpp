#include "core-imgui/Strategy.h"
#include "core/Loop.h"

#include "./strategies/ECSStrategy.h"
#include "services/yolo/YOLOSegmentor.h"

int main() {
  SegNetConfig DetectorConfig = {
      0.3,
      0.3,
      "../../../../models/second-best-n-640-seg.onnx",
      "../../../../models/class.names"};
  YOLOSegmentor net(DetectorConfig);

  CoreImGui::Strategy gui;
  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
