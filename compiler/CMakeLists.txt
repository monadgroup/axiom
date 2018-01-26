cmake_minimum_required(VERSION 3.1)
project(axiom_compiler)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES parser/TokenStream.cpp parser/Token.cpp parser/Parser.cpp

        ast/AssignExpression.h ast/AssignableExpression.h ast/Block.h ast/CallExpression.h ast/CastExpression.h ast/ControlExpression.h
        ast/Expression.cpp ast/Form.h ast/MathExpression.h ast/NoteExpression.h ast/NumberExpression.h ast/PostfixExpression.h
        ast/PropertyExpression.h ast/UnaryExpression.h ast/VariableExpression.h)

add_library(axiom_compiler ${SOURCE_FILES})
add_executable(axiom_test_compiler main.cpp ${SOURCE_FILES})
