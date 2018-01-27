#pragma once

#include <vector>
#include <memory>

#include "AssignableExpression.h"

namespace MaximAst {

    class LValueExpression : public Expression {
    public:
        std::vector<std::unique_ptr<AssignableExpression>> assignments;

        LValueExpression(SourcePos startPos, SourcePos endPos) : Expression(startPos, endPos) {}

        void appendString(std::stringstream &s) override {
            s << "(lvalue ";
            for (size_t i = 0; i < assignments.size(); i++) {
                assignments[i]->appendString(s);
                if (i != assignments.size() - 1) s << " ";
            }
            s << ")";
        }
    };

}
