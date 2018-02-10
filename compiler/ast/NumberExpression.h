#pragma once

#include "Expression.h"

namespace MaximAst {

    class NumberExpression : public Expression {
    public:
        float value;
        std::unique_ptr<Form> form;

        NumberExpression(float value, std::unique_ptr<Form> form, SourcePos start, SourcePos end)
            : Expression(start, end), value(value), form(std::move(form)) {}

        void appendString(std::ostream &s) override {
            s << "(number " << value << " ";
            form->appendString(s);
            s << ")";
        }
    };

}
