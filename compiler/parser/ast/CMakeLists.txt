set(SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/AssignableExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/AssignExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Block.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/CallExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/CastExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ControlExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Expression.h" "${CMAKE_CURRENT_SOURCE_DIR}/Expression.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Form.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/LValueExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/MathExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/NoteExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/NumberExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/PostfixExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/TupleExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/UnaryExpression.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/VariableExpression.h")

target_sources(maxim_parser PRIVATE ${SOURCE_FILES})
