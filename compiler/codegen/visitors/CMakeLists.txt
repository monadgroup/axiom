cmake_minimum_required(VERSION 3.4.3)

set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AssignVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CallVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/CastVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ExpressionVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/LValueVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/MathVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoteVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumberVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/PostfixVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/TupleVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnaryVisitor.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/VariableVisitor.cpp")

target_sources(maxim_codegen PRIVATE ${SOURCE_FILES})
