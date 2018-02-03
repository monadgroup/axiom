#pragma once

#include "Expression.h"
#include "LValueExpression.h"

namespace MaximAst {

    class PostfixExpression : public Expression {
    public:
        enum class Type {
            INCREMENT,
            DECREMENT
        };

        std::unique_ptr<LValueExpression> left;
        Type type;

        PostfixExpression(std::unique_ptr<LValueExpression> left, Type type, SourcePos start, SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type) {}

        void appendString(std::ostream &s) override {
            s << "(postfix ";
            switch (type) {
                case Type::INCREMENT:
                    s << "++";
                    break;
                case Type::DECREMENT:
                    s << "--";
                    break;
            }
            s << " ";
            left->appendString(s);
            s << ")";
        }
    };

}
