#pragma once

#include <SDL.h>

#include "events/Event.h"

class KeyPressedEvent : public Events::EventBase {
 public:
  std::string asciiSymbol;

  explicit KeyPressedEvent(std::string& asciiSymbol)
      : asciiSymbol(asciiSymbol) {}
};
