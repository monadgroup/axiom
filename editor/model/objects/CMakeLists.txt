set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Connection.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Control.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlSurface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MidiControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CustomNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExtractControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/GroupNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PortalControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RootSurface.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
