#pragma once

#include <cstdint>
#include <string>

#include "robot/Geometry.h"

namespace ae::capture {

struct WindowInfo {
  std::uint32_t windowId = 0;
  std::int32_t ownerPid = 0;
  std::string appName;
  std::string title;
  // Global logical desktop coordinates (top-left origin), as reported at
  // enumeration time. Windows move - query windowFrame() for the current
  // frame before converting capture coordinates to desktop coordinates.
  robot::LogicalRect frame;
};

}  // namespace ae::capture
