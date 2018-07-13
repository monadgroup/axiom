cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 17)
if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(SOURCE_FILES
        SurfaceMirBuilder.h SurfaceMirBuilder.cpp)

if (WIN32)
    set(COMPILERLIBNAME "compiler.lib")
else ()
    # not sure what clang uses on OS X. assuming gcc anyway :P
    set(COMPILERLIBNAME "libcompiler.a")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILER_LINK_LIBS "${CMAKE_SOURCE_DIR}/compiler/target/debug/${COMPILERLIBNAME}")
else ()
    set(COMPILER_LINK_LIBS "${CMAKE_SOURCE_DIR}/compiler/target/release/${COMPILERLIBNAME}")
endif ()
set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} z ffi)

if (WIN32)
    set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} ws2_32 userenv advapi32 psapi)
else ()
    set(COMPILER_LINK_LIBS ${COMPILER_LINK_LIBS} pthread dl curses)
endif ()

add_library(maxim_compiler ${SOURCE_FILES})
add_dependencies(maxim_compiler compiler)
target_link_libraries(maxim_compiler ${COMPILER_LINK_LIBS} llvm_axiom)

add_subdirectory(interface)
