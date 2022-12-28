set(APP_NAME AppAutomationEngine)
set(APP_FOLDER_NAME automation-engine)

set(SOURCES
        src/main.cpp
        src/strategies/ECSStrategy.h
        src/strategies/ImguiImplement.h
        src/components/PositionComponent.h
        src/systems/PositionSystem.h
        )

add_executable(${APP_NAME} ${SOURCES})

target_link_libraries(${APP_NAME} Core)
target_link_libraries(${APP_NAME} CoreImgui)
target_link_libraries(${APP_NAME} ECS)
