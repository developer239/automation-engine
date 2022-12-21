set(APP_NAME AppAutomationEngine)
set(APP_FOLDER_NAME automation-engine)

set(SOURCES
        src/main.cpp
        )

add_executable(${APP_NAME} ${SOURCES})

target_link_libraries(${APP_NAME} Loop)
