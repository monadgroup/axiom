cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SOURCE_FILES
        CommonColors.cpp
        FloatingValueEditor.cpp
        GlobalActions.cpp
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
