#pragma once

#include <ApplicationServices/ApplicationServices.h>

class Mouse {
  public:
    static Mouse& getInstance() {
      static Mouse instance;
      return instance;
    }

    void move(float x, float y);
    void click(CGMouseButton button, bool shouldPress);
    CGPoint getLocation();

  private:
    Mouse() = default;
    Mouse(const Mouse&) = delete;
    Mouse& operator=(const Mouse&) = delete;

    virtual void executeEvent(CGMouseButton button, CGEventType type, CGPoint location);
};
