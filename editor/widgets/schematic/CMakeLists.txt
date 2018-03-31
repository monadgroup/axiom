cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/SchematicCanvas.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SchematicPanel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/SchematicView.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
