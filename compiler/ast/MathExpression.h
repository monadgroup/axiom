#pragma once

#include "Expression.h"
#include "../common/OperatorType.h"

namespace MaximAst {

    class MathExpression : public Expression {
    public:
        std::unique_ptr<Expression> left;
        MaximCommon::OperatorType type;
        std::unique_ptr<Expression> right;

        MathExpression(std::unique_ptr<Expression> left, MaximCommon::OperatorType type, std::unique_ptr<Expression> right, SourcePos start,
                       SourcePos end)
                : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) {}

        void appendString(std::ostream &s) override {
            s << "(math " << MaximCommon::operatorType2String(type) << " ";
            left->appendString(s);
            s << " ";
            right->appendString(s);
            s << ")";
        }
    };

}
