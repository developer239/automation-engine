#include <sol/sol.hpp>

#include "core-imgui/Strategy.h"
#include "core/Loop.h"

#include "./strategies/ECSStrategy.h"

int main() {
  CoreImGui::Strategy gui;
  ECSStrategy ecs;

  sol::state lua;
  lua.open_libraries(sol::lib::base);

  lua.script(R"(
        function hello(name)
            print("Hello, " .. name .. "!")
        end
    )");

  auto hello = lua["hello"];
  hello("World");

  auto loop = Core::Loop({&gui, &ecs});
  loop.Run();
}
