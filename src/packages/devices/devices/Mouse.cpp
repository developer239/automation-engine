#include "./Mouse.h"

#include "utility/Delay.h"

namespace Devices {

void Mouse::Move(float x, float y) {
  CGPoint location = CGPointMake(x, y);
  ExecuteEvent(kCGMouseButtonLeft, kCGEventMouseMoved, location);
}

void Mouse::Click(CGMouseButton button, bool shouldPress) {
  CGPoint location = GetLocation();
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

  ExecuteEvent(button, typePress, location);
  ExecuteEvent(button, typeRelease, location);
}

CGPoint Mouse::GetLocation() {
  CGEventRef event = CGEventCreate(nullptr);
  CGPoint cursor = CGEventGetLocation(event);
  CFRelease(event);

  return CGPointMake(cursor.x, cursor.y);
}

void Mouse::ExecuteEvent(
    CGMouseButton button, CGEventType type, CGPoint location
) {
  CGEventRef event = CGEventCreateMouseEvent(nullptr, type, location, button);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);

  Utility::delay(25);
}

}  // namespace Devices
