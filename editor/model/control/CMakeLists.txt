cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeExtractControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeIOControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeMidiControl.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeNumControl.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
