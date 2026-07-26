#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <vector>

#include "capture/CapturedFrame.h"
#include "capture/WindowInfo.h"
#include "common/Error.h"
#include "pipeline/Mailbox.h"

namespace ae::capture {

using FrameMailbox = pipeline::Mailbox<CapturedFrame>;

// ScreenCaptureKit capture of a single window. Frames arrive on an internal
// dispatch queue and are published to the frame mailbox as retained pixel
// buffer handles - the capture callback does no conversion, inference, or
// rendering work. Non-copyable and non-movable because the stream delegate
// holds a pointer back into the instance.
class WindowCapture {
 public:
  WindowCapture();
  ~WindowCapture();
  WindowCapture(const WindowCapture&) = delete;
  WindowCapture& operator=(const WindowCapture&) = delete;
  WindowCapture(WindowCapture&&) = delete;
  WindowCapture& operator=(WindowCapture&&) = delete;

  // On-screen normal-layer windows with titles, front to back. Blocks briefly
  // on the ScreenCaptureKit enumeration; triggers the Screen Recording
  // permission prompt on first use.
  static std::expected<std::vector<WindowInfo>, Error> listWindows();

  // Starts streaming the target window into the mailbox, replacing any
  // running stream.
  std::expected<void, Error> start(
      const WindowInfo& target, FrameMailbox& mailbox, double maxFps = 60.0
  );

  void stop();

  [[nodiscard]] bool isRunning() const noexcept;

  [[nodiscard]] std::optional<WindowInfo> target() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace ae::capture
