#pragma once

#include <map>
#include <string>

namespace Devices {

class Keyboard {
 public:
  Keyboard() = default;

  Keyboard(const Keyboard&) = delete;

  Keyboard(Keyboard&&) = delete;

  Keyboard& operator=(const Keyboard&) = delete;

  Keyboard& operator=(Keyboard&&) = delete;

  static Keyboard& Instance() {
    static Keyboard instance;
    return instance;
  }

  void Type(const std::string& query);

  virtual void Click(char keyASCII);

  void Enter();

  void ArrowUp();

  void ArrowDown();

  void ArrowLeft();

  void ArrowRight();

  void Escape();

  void Space();

  void PressAndRelease(int key);

  int MapASCIIToVirtualKey(char key);

  char MapVirtualKeyToASCII(int key);

 private:
  static std::map<char, int> asciiToVirtualKey;
};

}  // namespace Devices
