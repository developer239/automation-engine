set(LIB_NAME GameLoopImgui)

add_library(${LIB_NAME} STATIC
        ./src/Test.cpp
        ./src/GUIStrategy.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PRIVATE Loop)

target_link_libraries(${LIB_NAME} PUBLIC Imgui)