#include "./Mouse.h"

#include "utility/Delay.h"

namespace Devices {

void Mouse::move(float x, float y) {
  CGPoint location = CGPointMake(x, y);
  executeEvent(kCGMouseButtonLeft, kCGEventMouseMoved, location);
}

void Mouse::click(CGMouseButton button, bool shouldPress) {
  CGPoint location = getLocation();
  CGEventType typePress;
  CGEventType typeRelease;

  switch (button) {
    case kCGMouseButtonLeft:
      typePress = shouldPress ? kCGEventLeftMouseDown : kCGEventLeftMouseUp;
      typeRelease = shouldPress ? kCGEventLeftMouseUp : kCGEventLeftMouseDown;
      break;

    case kCGMouseButtonRight:
      typePress = shouldPress ? kCGEventRightMouseDown : kCGEventRightMouseUp;
      typeRelease = shouldPress ? kCGEventRightMouseUp : kCGEventRightMouseDown;
      break;

    case kCGMouseButtonCenter:
      break;
  };

  executeEvent(button, typePress, location);
  executeEvent(button, typeRelease, location);
}

CGPoint Mouse::getLocation() {
  CGEventRef event = CGEventCreate(nullptr);
  CGPoint cursor = CGEventGetLocation(event);
  CFRelease(event);

  return CGPointMake(cursor.x, cursor.y);
}

void Mouse::executeEvent(
    CGMouseButton button, CGEventType type, CGPoint location
) {
  CGEventRef event = CGEventCreateMouseEvent(nullptr, type, location, button);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);

  Utility::delay(25);
}

}  // namespace Devices
