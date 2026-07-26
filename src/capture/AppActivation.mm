#include "capture/AppActivation.h"

#import <AppKit/AppKit.h>

namespace ae::capture {

bool activateApplication(const std::int32_t pid) {
  NSRunningApplication* app =
      [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
  if (app == nil || app.terminated) return false;
  return [app activateWithOptions:0];
}

}  // namespace ae::capture
