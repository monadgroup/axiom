cmake_minimum_required(VERSION 3.4.3)
project(axiom_compiler)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Including LLVM headers from ${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set(SOURCE_FILES util.h
        parser/TokenStream.cpp parser/Token.cpp parser/Parser.cpp
        codegen/MaximContext.cpp codegen/Midi.cpp codegen/Num.cpp codegen/Tuple.cpp codegen/NumType.cpp codegen/MidiType.cpp codegen/TupleType.cpp codegen/Converter.cpp codegen/Function.cpp codegen/Operator.cpp
        codegen/visitors/AssignVisitor.cpp codegen/visitors/CallVisitor.cpp codegen/visitors/CastVisitor.cpp codegen/visitors/ControlVisitor.cpp codegen/visitors/ExpressionVisitor.cpp codegen/visitors/MathVisitor.cpp codegen/visitors/NoteVisitor.cpp codegen/visitors/NumberVisitor.cpp codegen/visitors/PostfixVisitor.cpp codegen/visitors/TupleVisitor.cpp codegen/visitors/UnaryVisitor.cpp codegen/visitors/VariableVisitor.cpp
        ast/Expression.cpp)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(axiom_compiler ${SOURCE_FILES})
add_executable(axiom_test_compiler main.cpp ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(axiom_test_compiler ${llvm_libs})
