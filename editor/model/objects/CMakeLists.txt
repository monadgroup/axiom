set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Control.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlSurface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CustomNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/GroupNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RootSurface.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
