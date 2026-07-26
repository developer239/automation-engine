#pragma once

#include <CoreVideo/CoreVideo.h>

namespace ae::capture {

// Retaining RAII handle for a captured CVPixelBuffer. Copies retain, so a
// frame travels through the pipeline by reference to the same IOSurface-backed
// buffer and is never deep-copied.
class FrameHandle {
 public:
  FrameHandle() = default;

  explicit FrameHandle(CVPixelBufferRef buffer) : buffer_(buffer) {
    if (buffer_ != nullptr) CVPixelBufferRetain(buffer_);
  }

  FrameHandle(const FrameHandle& other) : buffer_(other.buffer_) {
    if (buffer_ != nullptr) CVPixelBufferRetain(buffer_);
  }

  FrameHandle& operator=(const FrameHandle& other) {
    if (this == &other) return *this;
    if (buffer_ != nullptr) CVPixelBufferRelease(buffer_);
    buffer_ = other.buffer_;
    if (buffer_ != nullptr) CVPixelBufferRetain(buffer_);
    return *this;
  }

  FrameHandle(FrameHandle&& other) noexcept : buffer_(other.buffer_) {
    other.buffer_ = nullptr;
  }

  FrameHandle& operator=(FrameHandle&& other) noexcept {
    if (this == &other) return *this;
    if (buffer_ != nullptr) CVPixelBufferRelease(buffer_);
    buffer_ = other.buffer_;
    other.buffer_ = nullptr;
    return *this;
  }

  ~FrameHandle() {
    if (buffer_ != nullptr) CVPixelBufferRelease(buffer_);
  }

  [[nodiscard]] CVPixelBufferRef get() const noexcept { return buffer_; }

  explicit operator bool() const noexcept { return buffer_ != nullptr; }

 private:
  CVPixelBufferRef buffer_ = nullptr;
};

}  // namespace ae::capture
