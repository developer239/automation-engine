cmake_minimum_required(VERSION 3.24)
project(AutomationEngine LANGUAGES C CXX OBJCXX)

if(NOT APPLE)
  message(FATAL_ERROR "automation-engine currently supports macOS only")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)

add_subdirectory(externals/robot-cpp)
add_subdirectory(externals/spdlog)
add_subdirectory(externals/sol2)

# Lua ships no CMake build; compile the 5.4 static library from the canonical
# source list (lua.c is the standalone interpreter, ltests.c/onelua.c are
# upstream dev artifacts - all three stay out of the library).
add_library(lua_static STATIC
    externals/lua/lapi.c
    externals/lua/lauxlib.c
    externals/lua/lbaselib.c
    externals/lua/lcode.c
    externals/lua/lcorolib.c
    externals/lua/lctype.c
    externals/lua/ldblib.c
    externals/lua/ldebug.c
    externals/lua/ldo.c
    externals/lua/ldump.c
    externals/lua/lfunc.c
    externals/lua/lgc.c
    externals/lua/linit.c
    externals/lua/liolib.c
    externals/lua/llex.c
    externals/lua/lmathlib.c
    externals/lua/lmem.c
    externals/lua/loadlib.c
    externals/lua/lobject.c
    externals/lua/lopcodes.c
    externals/lua/loslib.c
    externals/lua/lparser.c
    externals/lua/lstate.c
    externals/lua/lstring.c
    externals/lua/lstrlib.c
    externals/lua/ltable.c
    externals/lua/ltablib.c
    externals/lua/ltm.c
    externals/lua/lundump.c
    externals/lua/lutf8lib.c
    externals/lua/lvm.c
    externals/lua/lzio.c
)

# sol2 includes <lua/lua.h> when any include directory provides one, and
# Homebrew (pulled onto the include path by SDL3) ships an incompatible Lua
# under /opt/homebrew/include/lua. Mirror the vendored public headers into a
# lua/ shim directory that resolves first, so sol2 always compiles against the
# pinned 5.4 sources it links.
set(LUA_HEADER_SHIM_DIR ${CMAKE_BINARY_DIR}/lua-header-shim)
file(MAKE_DIRECTORY ${LUA_HEADER_SHIM_DIR}/lua)
foreach(LUA_PUBLIC_HEADER lua.h luaconf.h lauxlib.h lualib.h)
    configure_file(
        externals/lua/${LUA_PUBLIC_HEADER}
        ${LUA_HEADER_SHIM_DIR}/lua/${LUA_PUBLIC_HEADER}
        COPYONLY
    )
endforeach()

target_include_directories(lua_static PUBLIC externals/lua ${LUA_HEADER_SHIM_DIR})
target_compile_definitions(lua_static PUBLIC LUA_USE_MACOSX)

add_library(imgui_sdl3 STATIC
    externals/imgui/imgui.cpp
    externals/imgui/imgui_demo.cpp
    externals/imgui/imgui_draw.cpp
    externals/imgui/imgui_tables.cpp
    externals/imgui/imgui_widgets.cpp
    externals/imgui/backends/imgui_impl_sdl3.cpp
    externals/imgui/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui_sdl3 PUBLIC externals/imgui externals/imgui/backends)
target_link_libraries(imgui_sdl3 PUBLIC SDL3::SDL3)

add_subdirectory(src)
