cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ModuleBrowserPanel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ModulePreviewButton.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ModulePreviewCanvas.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ModulePreviewList.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
