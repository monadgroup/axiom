cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5Core REQUIRED)
get_target_property(QT5_INCLUDE_DIRS Qt5::Core INTERFACE_INCLUDE_DIRECTORIES)

set(SOURCE_FILES
        Control.cpp
        ControlGroup.cpp
        CustomNode.cpp
        GeneratableModuleClass.cpp
        GroupNode.cpp
        Jit.cpp
        ModuleRuntimeUnit.cpp
        HardControl.cpp
        Node.cpp
        IOControl.cpp
        IONode.cpp
        RootSurface.cpp
        Surface.cpp
        Runtime.cpp
        RuntimeUnit.cpp
        SoftControl.cpp
        ValueOperator.cpp)

include_directories(${LLVM_INCLUDE_DIRS} ${QT5_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_runtime ${SOURCE_FILES})
