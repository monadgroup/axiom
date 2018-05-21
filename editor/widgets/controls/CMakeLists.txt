cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/ExtractPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/KnobPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/PlugPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/SliderPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/TogglePainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExtractControlItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PortalControlItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MidiControlItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumControlItem.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
