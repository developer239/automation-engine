set(LIB_NAME Utility)

add_library(${LIB_NAME} STATIC
        ./utility/Delay.h
        ./utility/Delay.cpp
        ./utility/MatrixAsTexture.cpp
        ./utility/MatrixAsTexture.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PRIVATE
        Core
        Devices
        )
