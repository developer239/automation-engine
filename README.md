# Automation Engine

I created this project so that I have an opportunity to learn C++

It is goal is to provide a simple way to automate tasks on a computer by writing scripts in Lua.

### Requirements:

- `$ brew install cmake`
- `$ brew install sdl2`
- `$ brew install sdl2_ttf`
- `$ brew install sdl2_image`
- `$ brew install sdl2_mixer`

### Packages

- [Core](src/packages/core) - Responsible for game loop and SDL initialisation.
- [Core - ImGui](src/packages/core-imgui) - Strategy ImGui implementation so that ImGui can be used
  with [Core](/src/packages/core).
- [ECS](src/packages/ecs) - Custom minimalistic and probably not too performant entity component system.
- [Events](src/packages/events) - Custom minimalistic event bus implementation.
- [Logger](src/packages/logger) - Custom logger package.

### Tests

You can use CMake to run [googletest](https://github.com/google/googletest).