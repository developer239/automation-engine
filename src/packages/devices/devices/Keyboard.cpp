#include "./Keyboard.h"

#include <ApplicationServices/ApplicationServices.h>

namespace Devices {

void Keyboard::Type(const std::string& query) {
  for (char c : query) {
    Click(c);
  }
}

void Keyboard::Click(char keyASCII) {
  int key = MapASCIIToVirtualKey(keyASCII);
  PressAndRelease(key);
}

void Keyboard::ClickEnter() { PressAndRelease(52); }

void Keyboard::ArrowUp() { PressAndRelease(126); }

void Keyboard::ArrowDown() { PressAndRelease(125); }

void Keyboard::ArrowLeft() { PressAndRelease(123); }

void Keyboard::ArrowRight() { PressAndRelease(124); }

void Keyboard::PressAndRelease(int key) {
  CGEventSourceRef source =
      CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

  CGEventRef event = CGEventCreateKeyboardEvent(source, (CGKeyCode)key, true);
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);

  event = CGEventCreateKeyboardEvent(source, (CGKeyCode)key, false);
  CGEventPost(kCGHIDEventTap, event);

  CFRelease(event);
  CFRelease(source);
}

int Keyboard::MapASCIIToVirtualKey(char key) {
  auto it = asciiToVirtualKey.find(key);
  if (it == asciiToVirtualKey.end()) {
    throw std::invalid_argument("Unsupported key");
  }
  return it->second;
}

std::map<char, int> Keyboard::asciiToVirtualKey = {
    {'a', 0},
    {'s', 1},
    {'d', 2},
    {'f', 3},
    {'h', 4},
    {'g', 5},
    {'z', 6},
    {'x', 7},
    {'c', 8},
    {'v', 9},
    {'b', 11},
    {'q', 12},
    {'w', 13},
    {'e', 14},
    {'r', 15},
    {'y', 16},
    {'t', 17},
    {'1', 18},
    {'2', 19},
    {'3', 20},
    {'4', 21},
    {'6', 22},
    {'5', 23},
    {'=', 24},
    {'9', 25},
    {'7', 26},
    {'-', 27},
    {'8', 28},
    {'0', 29},
    {')', 30},
    {'o', 31},
    {'u', 32},
    {'(', 33},
    {'i', 34},
    {'p', 35},
    {'l', 37},
    {'j', 38},
    {39, 39},  // single quote
    {'k', 40},
    {';', 41},
    {'\\', 42},
    {',', 44},
    {'/', 45},
    {'n', 45},
    {'m', 46},
    {'.', 47},
    // {'TAB', 48},
    {' ', 49},
    {'`', 50},
};

  void Keyboard::ClickEscape() {
    PressAndRelease(53);
  }

}  // namespace Devices