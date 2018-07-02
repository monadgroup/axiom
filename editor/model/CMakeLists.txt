cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(SOURCE_FILES
        CloneReferenceMapper.cpp
        ConnectionWire.cpp
        HistoryList.cpp
        Library.cpp
        LibraryEntry.cpp
        ModelObject.cpp
        ModelRoot.cpp
        Pool.cpp
        PoolObject.cpp
        Project.cpp
        Value.cpp)

include_directories(${QT5_INCLUDE_DIRS})

add_library(axiom_model ${SOURCE_FILES})

add_subdirectory(actions)
add_subdirectory(grid)
add_subdirectory(objects)
