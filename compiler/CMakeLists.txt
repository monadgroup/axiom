cmake_minimum_required(VERSION 3.4.3)
project(axiom)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 17)

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror")
endif ()

set(SOURCE_FILES util.cpp
        common/FormType.cpp common/OperatorType.cpp common/ControlType.cpp
        parser/TokenStream.cpp parser/Token.cpp parser/Parser.cpp
        ast/Expression.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_common ${SOURCE_FILES})

add_subdirectory(codegen)
add_subdirectory(runtime)
