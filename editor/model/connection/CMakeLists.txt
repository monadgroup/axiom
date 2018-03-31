cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ConnectionSink.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ConnectionWire.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExtractConnectionSink.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MidiConnectionSink.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumConnectionSink.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
