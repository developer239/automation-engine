#pragma once

#include <ApplicationServices/ApplicationServices.h>

namespace Devices {

  class Mouse {
    public:
      Mouse() = default;

      Mouse(const Mouse&) = delete;

      Mouse& operator=(const Mouse&) = delete;

      static Mouse& getInstance() {
        static Mouse instance;
        return instance;
      }

      void move(float x, float y);

      void click(CGMouseButton button, bool shouldPress);

      CGPoint getLocation();

    private:
      virtual void executeEvent(CGMouseButton button, CGEventType type, CGPoint location);
  };

} // namespace Devices