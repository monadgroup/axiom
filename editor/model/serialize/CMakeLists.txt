set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ConnectionSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ConnectionSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ControlSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlSurfaceSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ControlSurfaceSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/HistorySerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/HistorySerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/LibrarySerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/LibrarySerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ModelObjectSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ModelObjectSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/NodeSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurfaceSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurfaceSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ProjectSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ProjectSerializer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ValueSerializer.h" "${CMAKE_CURRENT_SOURCE_DIR}/ValueSerializer.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
