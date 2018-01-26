#pragma once

#include "Expression.h"
#include "Form.h"

namespace MaximAst {

    class CastExpression : public Expression {
    public:
        Form target;
        std::unique_ptr<Expression> expr;
        bool isConvert;

        CastExpression(Form target, std::unique_ptr<Expression> expr, bool isConvert, SourcePos start, SourcePos end)
                : Expression(start, end), target(target), expr(std::move(expr)), isConvert(isConvert) { }
    };

}
