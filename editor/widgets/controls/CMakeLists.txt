cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/ExtractPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/KnobPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/PlugPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/SliderPainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/painters/TogglePainter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlItem.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExtractControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/IOControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MidiControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumControl.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
