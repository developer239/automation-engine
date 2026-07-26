#pragma once

#include <chrono>
#include <cstdint>

#include "capture/FrameHandle.h"

namespace ae::capture {

struct CapturedFrame {
  FrameHandle pixels;
  // Which window this frame came from. Frames outlive capture switches inside
  // the mailbox, so consumers must match this against their current target
  // instead of assuming the latest frame belongs to it.
  std::uint32_t windowId = 0;
  std::int32_t width = 0;   // device pixels
  std::int32_t height = 0;  // device pixels
  // Per-frame ScreenCaptureKit metadata, required to convert capture pixel
  // coordinates back to window points:
  //   surfacePoint = pixel / scaleFactor
  //   windowPoint  = (surfacePoint - contentOrigin) / contentScale
  // Zero when the OS omitted the attachments - consumers must refuse
  // coordinate-dependent work rather than guess.
  double scaleFactor = 0.0;   // device pixels per surface point (Retina: 2.0)
  double contentScale = 0.0;  // downscale applied to fit content in the buffer
  double contentX = 0.0;      // content origin within the surface, in points
  double contentY = 0.0;
  double contentWidth = 0.0;  // content size within the surface, in points
  double contentHeight = 0.0;
  // Presentation timestamp mapped onto steady_clock (not callback arrival
  // time), so frame age reflects how old the pixels actually are.
  std::chrono::steady_clock::time_point capturedAt;
};

}  // namespace ae::capture
