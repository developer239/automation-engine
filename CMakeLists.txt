cmake_minimum_required(VERSION 3.23)
project(CMake_SDL2_ImGui)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Find GoogleTest
add_subdirectory(externals/googletest)
enable_testing()

# Find ImGui
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/imgui/")
find_package(Imgui REQUIRED)

# Find SDL2
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/sdl2/")
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Include helpers
include(cmake-helpers/UseOpenCV.cmake)
include(cmake-helpers/UsePackage.cmake)

add_subdirectory(src)
