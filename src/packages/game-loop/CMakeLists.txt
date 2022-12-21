set(LIB_NAME Loop)

add_library(${LIB_NAME} STATIC
        src/GameLoop.cpp
        ./src/GameLoop.h
        src/GameRenderer.cpp
        ./src/GameRenderer.h
        ./src/GameStrategy.h
        src/GameWindow.cpp
        ./src/GameWindow.h)

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PUBLIC
        SDL2::SDL2
        SDL2::TTF
        SDL2::Image
        SDL2::Mixer)
