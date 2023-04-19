set(LIB_NAME Devices)
set(LIB_NAME_TEST ${LIB_NAME}Test)

add_library(${LIB_NAME} STATIC
        ./devices/Keyboard.cpp
        ./devices/Keyboard.h
        ./devices/Mouse.cpp
        ./devices/Mouse.h
        ./devices/Screen.cpp
        ./devices/Screen.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PUBLIC
        SDL2::SDL2
        SDL2::TTF
        SDL2::Image
        SDL2::Mixer)

target_link_libraries(${LIB_NAME} PRIVATE Utility)

target_link_libraries(${LIB_NAME} PUBLIC Imgui)
target_link_libraries(${LIB_NAME} PUBLIC "-framework Carbon")

# OpenCV start
target_link_libraries(${LIB_NAME} PUBLIC ${OpenCV_LIBS})

target_link_libraries(${LIB_NAME} PUBLIC
        ${tesseract_lib}
        ${leptonica_lib}
        ${TESSERACT_LIBRARIES}
        ${LEPTONICA_LIBRARIES}
        )

target_include_directories(${LIB_NAME} PUBLIC
        ${tesseract_include}
        ${leptonica_include})
# OpenCV end

add_test(AllTests${LIB_NAME} ${LIB_NAME_TEST})

set(SOURCES_TEST
        ./devices/Keyboard.cpp
        ./devices/Keyboard.h
        ./devices/Keyboard.test.cpp
        ./devices/Mouse.cpp
        ./devices/Mouse.h
        ./devices/Mouse.test.cpp
        ./devices/Screen.cpp
        ./devices/Screen.h
        ./devices/Screen.test.cpp
        )

add_executable(${LIB_NAME_TEST} ${SOURCES_TEST})

target_link_libraries(${LIB_NAME_TEST} PRIVATE
        gtest
        gmock
        gmock_main
        SDL2::SDL2
        SDL2::TTF
        SDL2::Image
        SDL2::Mixer)
target_link_libraries(${LIB_NAME_TEST} PUBLIC "-framework Carbon")
target_link_libraries(${LIB_NAME_TEST} PRIVATE
        gtest
        gmock
        gmock_main
        Utility)

# OpenCV start
target_link_libraries(${LIB_NAME_TEST} PUBLIC ${OpenCV_LIBS})

target_link_libraries(${LIB_NAME_TEST} PUBLIC
        ${tesseract_lib}
        ${leptonica_lib}
        ${TESSERACT_LIBRARIES}
        ${LEPTONICA_LIBRARIES}
        )

target_include_directories(${LIB_NAME_TEST} PUBLIC
        ${tesseract_include}
        ${leptonica_include})
# OpenCV end
