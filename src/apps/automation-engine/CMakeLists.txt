set(APP_NAME AppAutomationEngine)
set(APP_FOLDER_NAME automation-engine)

set(SOURCES
        src/events/MessageEvent.h
        src/layout/ImageStreamWindow.h
        src/layout/ImageStreamWindowControlsWindow.h
        src/layout/LoggingWindow.h
        src/layout/FPSWindow.h
        src/strategies/ECSStrategy.h
        src/systems/GUISystem/GUISystem.h
        src/systems/GUISystem/GUISystem.structs.h
        src/systems/GUISystem/IGUISystemWindow.h
        src/systems/ScreenSystem.h
        src/main.cpp
        src/services/FontManager.h
        src/components/TextLabelComponent.h
        src/components/BoundingBoxComponent.h
        src/systems/RenderTextSystem.h)

add_executable(${APP_NAME} ${SOURCES})

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/src/apps/${APP_FOLDER_NAME})

target_link_libraries(${APP_NAME} Core)
target_link_libraries(${APP_NAME} CoreImgui)
target_link_libraries(${APP_NAME} ECS)
target_link_libraries(${APP_NAME} Devices)
target_link_libraries(${APP_NAME} Utility)
target_link_libraries(${APP_NAME} Events)
