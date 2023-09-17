set(APP_NAME AppAutomationEngine)
set(APP_FOLDER_NAME automation-engine)

set(SOURCES
        ../../../externals/ImGuiFileDialog/ImGuiFileDialog.cpp
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
        src/components/TextLabelComponent.h
        src/components/BoundingBoxComponent.h
        src/systems/RenderTextSystem.h
        src/systems/RenderBoundingBoxSystem.h
        src/systems/ScriptingSystem.h
        src/layout/LoadScriptWindow.h
        src/events/ScriptFileSelectedEvent.h
        src/layout/ManageEntitiesWindow.h
        src/systems/RenderEditableComponentsGUISystem.h
        src/components/EditableComponent.h
        src/structs/Size.h
        src/structs/Color.h
        src/structs/Position.h
        src/components/DetectionComponent.h
        src/components/DetectContoursComponent.h
        src/systems/Detection/DetectContoursSystem.h
        src/components/DetectTextComponent.h
        src/systems/Detection/DetectionSystemBase.h
        src/events/KeyPressedEvent.h
        src/services/GlobalKeyboardListener.cpp
        src/components/DetectObjectsComponent.h src/systems/Detection/DetectObjectsSystem.h src/components/InstanceSegmentationComponent.h src/systems/Detection/InstanceSegmentationSystem.h src/components/SegmentMaskComponent.h src/systems/RenderSegmentMaskSystem.h src/systems/OdometerSystem.h src/components/OdometerComponent.h src/systems/CartographySystem.h src/layout/CartographyMapperWindow.h src/layout/CartographyMappedViewWindow.h src/services/ParallelTaskManager.h src/services/PathFinder.h)

add_executable(${APP_NAME} ${SOURCES})

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/src/apps/${APP_FOLDER_NAME})

target_link_libraries(${APP_NAME} Core)
target_link_libraries(${APP_NAME} CoreImgui)
target_link_libraries(${APP_NAME} ECS)
target_link_libraries(${APP_NAME} Devices)
target_link_libraries(${APP_NAME} Utility)
target_link_libraries(${APP_NAME} Events)
target_link_libraries(${APP_NAME} YOLO)

target_link_libraries(${APP_NAME} ${LUA_LIBRARY} ${Tesseract_LIBRARIES} ${Leptonica_LIBRARIES})
target_include_directories(${APP_NAME} PRIVATE ${LUA_INCLUDE_DIR} ${SOL2_INCLUDE_DIRS})

include_directories(${Tesseract_INCLUDE_DIRS} ${Leptonica_INCLUDE_DIRS})
link_directories(${Tesseract_LIBRARY_DIRS} ${Leptonica_LIBRARY_DIRS})
