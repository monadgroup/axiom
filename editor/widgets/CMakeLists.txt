set(CMAKE_AUTOMOC ON)

set(SOURCE_FILES
        CommonColors.cpp
        FloatingValueEditor.cpp
        GlobalActions.cpp
        InteractiveImport.cpp
        ItemResizer.cpp
        SpaceCompleter.cpp)

include_directories(../../ ${Qt5Widgets_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})

add_library(axiom_widgets ${SOURCE_FILES})

add_subdirectory(connection)
add_subdirectory(controls)
add_subdirectory(dock)
add_subdirectory(history)
add_subdirectory(layouts)
add_subdirectory(modulebrowser)
add_subdirectory(node)
add_subdirectory(surface)
add_subdirectory(windows)
