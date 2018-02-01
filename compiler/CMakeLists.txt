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
                 codegen/Context.cpp codegen/Form.cpp codegen/Function.cpp codegen/Parameter.cpp
                 codegen/types/FormType.cpp codegen/types/MidiType.cpp codegen/types/NumType.cpp codegen/types/RawNumType.cpp codegen/types/TupleType.cpp codegen/types/Type.cpp

        ast/AssignExpression.h ast/AssignableExpression.h ast/Block.h ast/CallExpression.h ast/CastExpression.h ast/ControlExpression.h
        ast/Expression.cpp ast/Form.h ast/MathExpression.h ast/NoteExpression.h ast/NumberExpression.h ast/PostfixExpression.h
        ast/PropertyExpression.h ast/UnaryExpression.h ast/VariableExpression.h ast/LValueExpression.h ast/TupleExpression.h)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(axiom_compiler ${SOURCE_FILES})
add_executable(axiom_test_compiler main.cpp ${SOURCE_FILES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(axiom_test_compiler ${llvm_libs})
