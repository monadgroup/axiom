cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/CustomNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/IONode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/GroupNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurface.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
