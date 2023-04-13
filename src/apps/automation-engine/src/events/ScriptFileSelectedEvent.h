#pragma once

#include <string>
#include <utility>

#include "events/Event.h"

class ScriptFileSelectedEvent : public Events::EventBase {
 public:
  std::string filePath;

  explicit ScriptFileSelectedEvent(std::string filePath)
      : filePath(std::move(filePath)) {}
};
