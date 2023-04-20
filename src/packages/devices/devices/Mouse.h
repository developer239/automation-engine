#pragma once

#include <ApplicationServices/ApplicationServices.h>

namespace Devices {

  class Mouse {
    public:
      Mouse() = default;

      Mouse(const Mouse&) = delete;

      Mouse& operator=(const Mouse&) = delete;

      static Mouse& Instance() {
        static Mouse instance;
        return instance;
      }

      void Move(float x, float y);

      // TODO: expose custom type
      void Click(CGMouseButton button, bool shouldPress);

      CGPoint GetLocation();

    private:
      virtual void ExecuteEvent(CGMouseButton button, CGEventType type, CGPoint location);
  };

} // namespace Devices
