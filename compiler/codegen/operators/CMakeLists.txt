cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES
        NumComparisonOperator.cpp NumFloatOperator.cpp NumIntOperator.cpp NumIntrinsicOperator.cpp
        NumLogicalOperator.cpp NumOperator.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_codegen_operators ${SOURCE_FILES})
