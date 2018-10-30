cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(AXIOM_VERSION 0.4.0)
set(AXIOM_VERSION_LIST 0,4,0)
add_definitions(-DAXIOM_VERSION="${AXIOM_VERSION}\\0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

add_subdirectory(compiler)
add_subdirectory(editor)
