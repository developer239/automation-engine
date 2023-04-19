set(LIB_NAME YOLO)

add_library(${LIB_NAME} STATIC
        ./yolo/YOLODetector.h
        ./yolo/YOLODetector.cpp
        ./yolo/YOLOSegmentor.h
        )

target_include_directories(${LIB_NAME} INTERFACE .)

target_link_libraries(${LIB_NAME} PUBLIC ${ONNXRUNTIME_LIBRARY})
target_include_directories(${LIB_NAME} PUBLIC ${ONNXRUNTIME_INCLUDE_DIR})

target_link_libraries(${LIB_NAME} PUBLIC ${OpenCV_LIBS})
