cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ads_globals.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockAreaWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockContainerWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockManager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockOverlay.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockSplitter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockStateSerialization.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockWidget.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/DockWidgetTitleBar.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/FloatingDockContainer.cpp")

target_sources(axiom_widgets PRIVATE ${SOURCE_FILES})
