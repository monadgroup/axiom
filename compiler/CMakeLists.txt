cmake_minimum_required(VERSION 3.4.3)
project(axiom)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOCC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES util.h
        common/FormType.cpp common/OperatorType.cpp common/ControlType.cpp
        parser/TokenStream.cpp parser/Token.cpp parser/Parser.cpp
        ast/Expression.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

#add_library(axiom_compiler ${SOURCE_FILES})
add_executable(axiom_compiler_test ${SOURCE_FILES} main.cpp)
add_subdirectory(codegen)
target_link_libraries(axiom_compiler_test Qt5::Widgets maxim_codegen)
set_target_properties(axiom_compiler_test PROPERTIES LINKER_LANGUAGE CXX)

add_library(axiom_compiler )
