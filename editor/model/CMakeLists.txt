cmake_minimum_required(VERSION 3.4.3)
project(axiom)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Core REQUIRED)
get_target_property(QT5_INCLUDE_DIRS Qt5::Core INTERFACE_INCLUDE_DIRECTORIES)

set(SOURCE_FILES
        GridItem.cpp
        GridSurface.cpp
        Library.cpp
        LibraryEntry.cpp
        Project.cpp
        Ref.cpp)

include_directories(${LLVM_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(axiom_model ${SOURCE_FILES})

add_subdirectory(connection)
add_subdirectory(control)
add_subdirectory(history)
add_subdirectory(node)
add_subdirectory(schematic)
