cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AddNodeMenu.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurfaceCanvas.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurfacePanel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurfaceView.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
