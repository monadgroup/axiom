cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(SOURCE_FILES Hookable.cpp SharedHookable.cpp)

add_library(axiom_common ${SOURCE_FILES})
