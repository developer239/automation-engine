#include "core-imgui/Strategy.h"
#include "core/Loop.h"

#include "./strategies/ECSStrategy.h"
#include "yolo/YOLOSegmentor.h"

int main() {
  YOLO::SegNetConfig DetectorConfig = {
      0.3,
      0.3,
      "../../../../models/second-best-n-640-seg.onnx",
      "../../../../models/class.names"};
  YOLO::YOLOSegmentor net(DetectorConfig);

  CoreImGui::Strategy gui;
  ECSStrategy ecs;

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
