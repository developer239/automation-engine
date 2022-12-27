# Automation Engine

I created this project so that I have an opportunity to learn C++

It is goal is to provide a simple way to automate tasks on a computer by writing scripts in Lua.

### Requirements:

- `$ brew install cmake`
- `$ brew install opencv`
- `$ brew install sdl2`
- `$ brew install sdl2_ttf`
- `$ brew install sdl2_image`
- `$ brew install sdl2_mixer`

### Packages

- [Core](src/packages/core) - Responsible for game loop and SDL initialisation.
- [Core - ImGui](src/packages/core-imgui) - Strategy ImGui implementation so that ImGui can be used
  with [Core](/src/packages/core).
- [Devices](src/packages/devices) - Hardware devices helpful for automation.
- [ECS](src/packages/ecs) - Custom minimalistic and probably not too performant entity component system.
- [Events](src/packages/events) - Custom minimalistic event bus implementation.
- [Logger](src/packages/logger) - Custom logger package.
- [Utility](src/packages/logger) - Utility functions used in packages and apps.

### Tests

You can use CMake to Run [googletest](https://github.com/google/googletest). (note: running all tests doesn't seem to work at the mean time)
