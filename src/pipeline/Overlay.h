#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "pipeline/Mailbox.h"

namespace ae::pipeline {

struct OverlayColor {
  std::uint8_t r = 255;
  std::uint8_t g = 255;
  std::uint8_t b = 255;
  std::uint8_t a = 255;
};

// All overlay coordinates are capture pixel space (the same space scripts see
// detections and frame dimensions in); the UI scales them to the displayed
// image rectangle when drawing.

struct OverlayBox {
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;
  OverlayColor color;
  float thickness = 2.0F;
  bool filled = false;
};

struct OverlayLine {
  float x1 = 0;
  float y1 = 0;
  float x2 = 0;
  float y2 = 0;
  OverlayColor color;
  float thickness = 2.0F;
};

struct OverlayText {
  float x = 0;
  float y = 0;
  std::string text;
  OverlayColor color;
};

using OverlayCommand = std::variant<OverlayBox, OverlayLine, OverlayText>;

// One control tick's drawing output. Replaced wholesale every tick - overlays
// carry no retained state or identity, they are a command buffer.
struct OverlaySnapshot {
  std::uint64_t frameSeq = 0;
  std::vector<OverlayCommand> commands;
};

using OverlayMailbox = Mailbox<OverlaySnapshot>;

}  // namespace ae::pipeline
