cmake_minimum_required(VERSION 3.1)
project(axiom_compiler)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES parser/TokenStream.cpp parser/Token.cpp)

add_library(axiom_compiler ${SOURCE_FILES})
add_executable(axiom_test_compiler main.cpp ${SOURCE_FILES})
