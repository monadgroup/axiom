cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(SOURCE_FILES
        SurfaceMirBuilder.h SurfaceMirBuilder.cpp)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILER_TARGET_DIRECTORY "${CMAKE_SOURCE_DIR}/compiler/target/debug")
else ()
    set(COMPILER_TARGET_DIRECTORY "${CMAKE_SOURCE_DIR}/compiler/target/release")
endif ()

if (WIN32)
    set(COMPILER_LINK_LIBS "${COMPILER_TARGET_DIRECTORY}/compiler.lib" ws2_32 userenv advapi32 psapi)
else ()
    set(COMPILER_LINK_LIBS "${COMPILER_TARGET_DIRECTORY}/libcompiler.a" pthread dl curses)
endif ()
set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} z ffi)

add_library(maxim_compiler ${SOURCE_FILES})
add_dependencies(maxim_compiler compiler)
target_link_libraries(maxim_compiler ${COMPILER_LINK_LIBS} llvm_axiom)

add_subdirectory(interface)
