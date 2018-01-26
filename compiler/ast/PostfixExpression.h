#pragma once

#include "Expression.h"

namespace MaximAst {

    class PostfixExpression : public Expression {
    public:
        enum class Type {
            INCREMENT,
            DECREMENT
        };

        std::unique_ptr<AssignableExpression> left;
        Type type;

        PostfixExpression(std::unique_ptr<AssignableExpression> left, Type type, SourcePos start, SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type) { }
    };

}
