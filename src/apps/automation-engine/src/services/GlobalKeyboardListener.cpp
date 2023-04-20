#include <iostream>

#include "../events/KeyPressedEvent.h"
#include "./GlobalKeyboardListener.h"
#include "devices/Keyboard.h"
#include "events/Bus.h"

GlobalKeyboardListener::GlobalKeyboardListener() {
  eventTap = CGEventTapCreate(
      kCGSessionEventTap,
      kCGHeadInsertEventTap,
      kCGEventTapOptionDefault,
      CGEventMaskBit(kCGEventKeyUp),
      KeyEventCallback,
      this
  );

  if (eventTap) {
    runLoopSource =
        CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        runLoopSource,
        kCFRunLoopCommonModes
    );
    CGEventTapEnable(eventTap, true);
  }
}

GlobalKeyboardListener::~GlobalKeyboardListener() {
  if (eventTap) {
    CGEventTapEnable(eventTap, false);
    CFRunLoopRemoveSource(
        CFRunLoopGetCurrent(),
        runLoopSource,
        kCFRunLoopCommonModes
    );
    CFRelease(runLoopSource);
    CFRelease(eventTap);
  }
}

// TODO: only listen to registered keys and combinations
CGEventRef GlobalKeyboardListener::KeyEventCallback(
    CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon
) {
  if (type == kCGEventKeyUp) {
    int keyCode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    auto symbol =
        Devices::Keyboard::Instance().MapVirtualKeyToASCII((int) keyCode);
    auto symbolString = std::string(1, symbol);

    Events::Bus::Instance().EmitEvent<KeyPressedEvent>(symbolString);
  }

  return event;
}
