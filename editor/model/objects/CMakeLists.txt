set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/RootSurface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Surface.cpp")

target_sources(axiom_model PRIVATE ${SOURCE_FILES})
