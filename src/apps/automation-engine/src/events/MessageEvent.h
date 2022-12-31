#pragma once

#include <string>
#include <utility>

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

  explicit MessageEvent(std::string message, MessageLevel level = INFO)
      : message(std::move(message)), level(level) {}
};
