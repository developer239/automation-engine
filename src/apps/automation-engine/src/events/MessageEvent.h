#pragma once

#include <string>

#include "events/Event.h"

enum MessageLevel {
  INFO,
  WARNING,
  ERROR,
};

class MessageEvent : public Events::EventBase {
 public:
  std::string message;
  MessageLevel level = INFO;
};
