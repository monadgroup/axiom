#pragma once

#include "Expression.h"

namespace MaximAst {

    class AssignableExpression : public Expression {
    public:
        AssignableExpression(SourcePos startPos, SourcePos endPos) : Expression(startPos, endPos) { }
    };

}
