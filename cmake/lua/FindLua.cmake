find_path(LUA_INCLUDE_DIR NAMES lua.h PATH_SUFFIXES lua)
find_library(LUA_LIBRARY NAMES lua)

if (LUA_INCLUDE_DIR AND LUA_LIBRARY)
    set(LUA_FOUND TRUE)
else ()
    set(LUA_FOUND FALSE)
endif ()

if (LUA_FOUND)
    message(STATUS "Found Lua: ${LUA_LIBRARY}")
else ()
    message(FATAL_ERROR "Lua not found")
endif ()

mark_as_advanced(LUA_INCLUDE_DIR LUA_LIBRARY)
