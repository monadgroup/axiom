cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/GroupSchematic.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/LibrarySchematic.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RootSchematic.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Schematic.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
