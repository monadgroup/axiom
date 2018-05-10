cmake_minimum_required(VERSION 3.4.3)
project(axiom)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif()

find_package(Qt5Core REQUIRED)
get_target_property(QT5_INCLUDE_DIRS Qt5::Core INTERFACE_INCLUDE_DIRECTORIES)

set(SOURCE_FILES
        Event.h
        ConnectionWire.cpp
        HistoryList.cpp
        Hookable.cpp
        ModelObject.cpp
        ModelRoot.cpp
        Pool.cpp
        PoolObject.cpp
        Project.cpp)

include_directories(${LLVM_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(axiom_model ${SOURCE_FILES})

add_subdirectory(actions)
add_subdirectory(grid)
add_subdirectory(objects)
