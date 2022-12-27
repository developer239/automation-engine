# TODO: remove duplication between lib and tests
# TODO: create function to generate package configuration

set(LIB_NAME Devices)
set(LIB_NAME_TEST ${LIB_NAME}Test)

add_library(${LIB_NAME} STATIC
        ./devices/Keyboard.cpp
        ./devices/Keyboard.h
        ./devices/Mouse.cpp
        ./devices/Mouse.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PRIVATE Utility)

target_link_libraries(${LIB_NAME} PUBLIC Imgui)

add_test(AllTests${LIB_NAME} ${LIB_NAME})

set(SOURCES_TEST
        ./devices/Keyboard.cpp
        ./devices/Keyboard.h
        ./devices/Keyboard.test.cpp
        ./devices/Mouse.cpp
        ./devices/Mouse.h
        ./devices/Mouse.test.cpp
        )

add_executable(${LIB_NAME_TEST} ${SOURCES_TEST})
target_link_libraries(${LIB_NAME_TEST} PUBLIC "-framework Carbon")
target_link_libraries(${LIB_NAME_TEST} PRIVATE
        gtest
        gmock
        gmock_main
        Utility)
