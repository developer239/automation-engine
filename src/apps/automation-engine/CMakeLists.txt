set(APP_NAME AppAutomationEngine)
set(APP_FOLDER_NAME automation-engine)

set(SOURCES
        src/main.cpp
        src/strategies/ECSStrategy.h
        src/strategies/ImguiStrategy.h
        src/components/PositionComponent.h
        src/systems/PositionSystem.h
        src/systems/DebuggerSystem.h)

add_executable(${APP_NAME} ${SOURCES})

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/src/apps/${APP_FOLDER_NAME})

target_link_libraries(${APP_NAME} Core)
target_link_libraries(${APP_NAME} CoreImgui)
target_link_libraries(${APP_NAME} ECS)
