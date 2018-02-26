cmake_minimum_required(VERSION 3.4.3)
project(axiom)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES
        AssignVisitor.cpp CallVisitor.cpp CastVisitor.cpp ControlVisitor.cpp ExpressionVisitor.cpp
        LValueVisitor.cpp MathVisitor.cpp NoteVisitor.cpp NumberVisitor.cpp PostfixVisitor.cpp TupleVisitor.cpp
        UnaryVisitor.cpp VariableVisitor.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(maxim_codegen_visitors ${SOURCE_FILES})
