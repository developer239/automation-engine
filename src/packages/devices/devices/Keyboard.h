#pragma once

#include <map>
#include <string>

class Keyboard {
 public:
  Keyboard() = default;
  Keyboard(const Keyboard&) = delete;
  Keyboard(Keyboard&&) = delete;
  Keyboard& operator=(const Keyboard&) = delete;
  Keyboard& operator=(Keyboard&&) = delete;

  static Keyboard& GetInstance() {
    static Keyboard instance;
    return instance;
  }

  void Type(const std::string& query);

  virtual void Click(char keyASCII);

  void ClickEnter();

  void ArrowUp();

  void ArrowDown();

  void ArrowLeft();

  void ArrowRight();

  void PressAndRelease(int key);

  int MapASCIIToVirtualKey(char key);

 private:
  static std::map<char, int> asciiToVirtualKey;
};
