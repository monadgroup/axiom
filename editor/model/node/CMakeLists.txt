cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/CustomNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/IONode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/JunctionNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ModuleNode.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NodeSurface.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
