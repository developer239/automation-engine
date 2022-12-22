# TODO: remove duplication between lib and tests
# TODO: create function to generate package configuration

set(LIB_NAME Core)
set(LIB_NAME_TEST ${LIB_NAME}Test)

add_library(${LIB_NAME} STATIC
        core/Loop.cpp
        core/Loop.h
        core/Renderer.cpp
        core/Renderer.h
        core/IStrategy.h
        core/Window.cpp
        core/Window.h)

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PUBLIC
        SDL2::SDL2
        SDL2::TTF
        SDL2::Image
        SDL2::Mixer)

add_test(AllTests${LIB_NAME} ${LIB_NAME})

set(SOURCES_TEST
        ./core/IStrategy.test.cpp
        core/Loop.cpp
        core/Loop.h
        core/Renderer.cpp
        core/Renderer.h
        core/IStrategy.h
        core/Window.cpp
        core/Window.h)
add_executable(${LIB_NAME_TEST} ${SOURCES_TEST})
target_link_libraries(${LIB_NAME_TEST} PRIVATE
        gtest
        gmock
        gmock_main
        SDL2::SDL2
        SDL2::TTF
        SDL2::Image
        SDL2::Mixer)
