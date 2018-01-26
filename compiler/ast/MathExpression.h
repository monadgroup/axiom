#pragma once

#include "Expression.h"

namespace MaximAst {

    class MathExpression : public Expression {
    public:
        enum class Type {
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
            POWER,

            BITWISE_AND,
            BITWISE_OR,
            BITWISE_XOR,
            LOGICAL_AND,
            LOGICAL_OR,
            LOGICAL_EQUAL,
            LOGICAL_NOT_EQUAL,
            LOGICAL_GT,
            LOGICAL_LT,
            LOGICAL_GTE,
            LOGICAL_LTE
        };

        std::unique_ptr<Expression> left;
        Type type;
        std::unique_ptr<Expression> right;

        MathExpression(std::unique_ptr<Expression> left, Type type, std::unique_ptr<Expression> right, SourcePos start, SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) { }
    };

}
