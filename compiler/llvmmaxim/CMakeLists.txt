cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})

if(APPLE)
    add_definitions(-DAPPLE)
endif()

add_library(llvm_axiom LLVMMaxim.cpp)
