#include "capture/WindowQueries.h"

#include <CoreGraphics/CoreGraphics.h>

namespace ae::capture {

namespace {

// Borrowed CFDictionary values; the owning array keeps them alive.
std::int32_t intValue(CFDictionaryRef dict, CFStringRef key) {
  const auto* number =
      static_cast<CFNumberRef>(CFDictionaryGetValue(dict, key));
  if (number == nullptr) return 0;
  std::int32_t value = 0;
  CFNumberGetValue(number, kCFNumberSInt32Type, &value);
  return value;
}

}  // namespace

std::optional<robot::LogicalRect> windowFrame(const std::uint32_t windowId) {
  CFArrayRef windows = CGWindowListCopyWindowInfo(
      kCGWindowListOptionIncludingWindow, windowId
  );
  if (windows == nullptr) return std::nullopt;

  std::optional<robot::LogicalRect> frame;
  if (CFArrayGetCount(windows) > 0) {
    const auto* info =
        static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows, 0));
    const auto* boundsDict = static_cast<CFDictionaryRef>(
        CFDictionaryGetValue(info, kCGWindowBounds)
    );
    CGRect bounds = CGRectZero;
    if (boundsDict != nullptr &&
        CGRectMakeWithDictionaryRepresentation(boundsDict, &bounds)) {
      frame = robot::LogicalRect{
          .origin = {.x = bounds.origin.x, .y = bounds.origin.y},
          .size = {.width = bounds.size.width, .height = bounds.size.height},
      };
    }
  }

  CFRelease(windows);
  return frame;
}

std::uint32_t frontmostWindowId() {
  CFArrayRef windows = CGWindowListCopyWindowInfo(
      kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
      kCGNullWindowID
  );
  if (windows == nullptr) return 0;

  std::uint32_t windowId = 0;
  for (CFIndex i = 0; i < CFArrayGetCount(windows); ++i) {
    const auto* info =
        static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windows, i));
    if (intValue(info, kCGWindowLayer) != 0) continue;
    windowId = static_cast<std::uint32_t>(intValue(info, kCGWindowNumber));
    break;
  }

  CFRelease(windows);
  return windowId;
}

}  // namespace ae::capture
