#pragma once

#include <SDL.h>

#include "events/Event.h"

class KeyPressedEvent : public Events::EventBase {
 public:
  SDL_Event sdlEvent;

  explicit KeyPressedEvent(SDL_Event sdlEvent) { this->sdlEvent = sdlEvent; }
};
