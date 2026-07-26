#pragma once

#include <cstdint>
#include <optional>

#include "robot/Geometry.h"

namespace ae::capture {

// Current global-logical frame of a window, or nullopt when it no longer
// exists on screen. Backed by CGWindowList, safe to call from any thread.
std::optional<robot::LogicalRect> windowFrame(std::uint32_t windowId);

// Id of the frontmost normal-layer window. Backed by CGWindowList (the list
// is ordered front to back), safe to call from any thread; 0 when no normal
// window is on screen. Window-level, not application-level: another window of
// the same app being frontmost does not count as the target having focus.
std::uint32_t frontmostWindowId();

}  // namespace ae::capture
