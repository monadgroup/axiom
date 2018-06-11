#pragma once

#include "AssignableExpression.h"

namespace MaximAst {

    class VariableExpression : public AssignableExpression {
    public:
        std::string name;

        VariableExpression(std::string name, SourcePos start, SourcePos end)
            : AssignableExpression(start, end), name(std::move(name)) {}

        void appendString(std::ostream &s) override {
            s << "(var " << name << ")";
        }
    };

}
