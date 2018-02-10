#pragma once

#include "Expression.h"
#include "Form.h"

namespace MaximAst {

    class CastExpression : public Expression {
    public:
        std::unique_ptr<Form> target;
        std::unique_ptr<Expression> expr;
        bool isConvert;

        CastExpression(std::unique_ptr<Form> target, std::unique_ptr<Expression> expr, bool isConvert, SourcePos start,
                       SourcePos end)
            : Expression(start, end), target(std::move(target)), expr(std::move(expr)), isConvert(isConvert) {}

        void appendString(std::ostream &s) override {
            s << "(" << (isConvert ? "convert" : "cast") << " ";
            target->appendString(s);
            s << " ";
            expr->appendString(s);
            s << ")";
        }
    };

}
