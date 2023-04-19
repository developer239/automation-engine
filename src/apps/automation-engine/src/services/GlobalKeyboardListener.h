#pragma once

#include <CoreGraphics/CoreGraphics.h>
#include <SDL.h>
#include <vector>
#include "core/IStrategy.h"

class GlobalKeyboardListener {
 public:
  explicit GlobalKeyboardListener();
  ~GlobalKeyboardListener();

  static CGEventRef KeyEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);

 private:
  CFMachPortRef eventTap;
  CFRunLoopSourceRef runLoopSource;
};
