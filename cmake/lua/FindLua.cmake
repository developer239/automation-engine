# FindLua.cmake

set(LUA_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/lua")

set(LUA_INCLUDE_DIR ${LUA_SOURCE_DIR})

file(GLOB LUA_SOURCES ${LUA_SOURCE_DIR}/*.c)
list(REMOVE_ITEM LUA_SOURCES "${LUA_SOURCE_DIR}/lua.c")

add_library(Lua STATIC ${LUA_SOURCES})
target_include_directories(Lua PUBLIC ${LUA_INCLUDE_DIR})

set(LUA_FOUND TRUE)
set(LUA_LIBRARY Lua)

if(LUA_FOUND)
    message(STATUS "Found Lua: ${LUA_LIBRARY}")
else()
    message(FATAL_ERROR "Lua not found")
endif()

mark_as_advanced(LUA_INCLUDE_DIR)
