cmake_minimum_required(VERSION 3.23)
project(AutomationEngine)

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

# Find Lua
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/lua/")
find_package(Lua REQUIRED)

# Find Sol2
set(SOL2_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/externals/sol2/include)

# Find Tesseract
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/tesseract/")

find_package(Tesseract REQUIRED)
find_package(Leptonica REQUIRED)

# ONNX
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/onnx/")
set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/externals/onnxruntime-osx-arm64-1.12.1")

find_package(ONNXRUNTIME REQUIRED)

add_subdirectory(src)
