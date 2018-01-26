#pragma once

#include "Expression.h"

namespace MaximAst {

    class NumberExpression : public Expression {
    public:
        float value;
        Form form;

        NumberExpression(float value, Form form, SourcePos start, SourcePos end)
                : Expression(start, end), value(value), form(form) { }
    };

}
