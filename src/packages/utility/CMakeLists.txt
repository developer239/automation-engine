# TODO: remove duplication between lib and tests
# TODO: create function to generate package configuration

set(LIB_NAME Utility)
set(LIB_NAME_TEST ${LIB_NAME}Test)

add_library(${LIB_NAME} STATIC
        ./utility/Delay.h
        ./utility/Delay.cpp
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PRIVATE Core)

target_link_libraries(${LIB_NAME} PUBLIC Imgui)

add_test(AllTests${LIB_NAME} ${LIB_NAME})

set(SOURCES_TEST
        ./utility/Delay.h
        ./utility/Delay.cpp
        ./utility/Delay.test.cpp)

add_executable(${LIB_NAME_TEST} ${SOURCES_TEST})
target_link_libraries(${LIB_NAME_TEST} PRIVATE
        gtest
        gmock
        gmock_main)
