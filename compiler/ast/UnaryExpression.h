#pragma once

#include "Expression.h"

namespace MaximAst {

    class UnaryExpression : public Expression {
    public:
        enum class Type {
            POSITIVE,
            NEGATIVE,
            NOT
        };

        Type type;
        std::unique_ptr<Expression> expr;

        UnaryExpression(Type type, std::unique_ptr<Expression> expr, SourcePos start, SourcePos end)
            : Expression(start, end), type(type), expr(std::move(expr)) {}

        void appendString(std::ostream &s) override {
            s << "(unary ";
            switch (type) {
                case Type::POSITIVE:
                    s << "+";
                    break;
                case Type::NEGATIVE:
                    s << "-";
                    break;
                case Type::NOT:
                    s << "!";
                    break;
            }
            s << " ";
            expr->appendString(s);
            s << ")";
        }
    };

}
