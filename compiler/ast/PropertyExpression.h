#pragma once

#include <memory>
#include <utility>

#include "AssignableExpression.h"

namespace MaximAst {

    class PropertyExpression : public AssignableExpression {
    public:
        std::unique_ptr<AssignableExpression> base;
        std::string property;

        PropertyExpression(std::unique_ptr<AssignableExpression> base, std::string property, SourcePos start,
                           SourcePos end)
                : AssignableExpression(start, end), base(std::move(base)), property(std::move(property)) {}

        void appendString(std::ostream &s) override {
            s << "(prop " << property << " ";
            base->appendString(s);
            s << ")";
        }
    };

}
