add_library(automation_engine_core STATIC
    capture/AppActivation.mm
    capture/WindowCapture.mm
    capture/WindowQueries.cpp
    control/ControlThread.cpp
    control/InputController.cpp
    scripting/ScriptRuntime.cpp
    ui/UiApp.cpp
)

target_include_directories(automation_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(automation_engine_core PRIVATE -Wall -Wextra)
target_compile_definitions(automation_engine_core PUBLIC SOL_ALL_SAFETIES_ON=1)

set_source_files_properties(
    capture/AppActivation.mm
    capture/WindowCapture.mm
    PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
)

# PUBLIC: types appear in this library's headers (robot geometry in
# WindowInfo.h, sol2/lua in ScriptRuntime.h, spdlog in LogRingSink.h,
# CoreVideo in FrameHandle.h). PRIVATE: implementation-only.
target_link_libraries(automation_engine_core
    PUBLIC
        robot::robot
        spdlog::spdlog
        sol2::sol2
        lua_static
        "-framework CoreVideo"
    PRIVATE
        imgui_sdl3
        SDL3::SDL3
        "-framework ScreenCaptureKit"
        "-framework CoreMedia"
        "-framework CoreGraphics"
        "-framework AppKit"
)

add_executable(automation-engine main.cpp)
target_link_libraries(automation-engine PRIVATE automation_engine_core)
