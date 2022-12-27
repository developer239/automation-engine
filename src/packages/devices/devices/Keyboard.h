#pragma once

#include <map>
#include <string>

class Keyboard {
 public:
  static void Type(const std::string& query);

  static void Click(char keyASCII);

  static void ClickEnter();

  static void ArrowUp();

  static void ArrowDown();

  static void ArrowLeft();

  static void ArrowRight();

  static void PressAndRelease(int key);

  static int MapASCIIToVirtualKey(char key);

 private:
  static std::map<char, int> asciiToVirtualKey;
};
