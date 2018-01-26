#pragma once

#include "Expression.h"

namespace MaximAst {

    class AssignExpression : public Expression {
    public:
        enum class Type {
            ASSIGN,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            MODULO,
            POWER
        };

        std::unique_ptr<AssignableExpression> left;
        Type type;
        std::unique_ptr<Expression> right;

        AssignExpression(std::unique_ptr<AssignableExpression> left, Type type, std::unique_ptr<Expression> right, SourcePos start, SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) { }
    };

}
