cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(CMAKE_AUTOMOCC ON)
set(CMAKE_AUTORCC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILER_LINK_LIBS "${CMAKE_SOURCE_DIR}/compiler/target/debug/compiler.lib")
else ()
    set(COMPILER_LINK_LIBS "${CMAKE_SOURCE_DIR}/compiler/target/release/compiler.lib")
endif ()
set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} z ffi)

if (WIN32)
    set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} ws2_32 userenv advapi32 psapi)
endif ()

add_library(maxim_compiler)
add_dependencies(maxim_compiler compiler)
target_link_libraries(maxim_compiler ${COMPILER_LINK_LIBS})

add_subdirectory(interface)
