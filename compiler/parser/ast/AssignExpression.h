#pragma once

#include "Expression.h"
#include "LValueExpression.h"
#include "../../common/OperatorType.h"

namespace MaximAst {

    class AssignExpression : public Expression {
    public:
        std::unique_ptr<LValueExpression> left;
        MaximCommon::OperatorType type;
        std::unique_ptr<Expression> right;

        AssignExpression(std::unique_ptr<LValueExpression> left, MaximCommon::OperatorType type,
                         std::unique_ptr<Expression> right,
                         SourcePos start, SourcePos end)
            : Expression(start, end), left(std::move(left)), type(type), right(std::move(right)) {}

        void appendString(std::ostream &s) override {
            s << "(assign " << MaximCommon::operatorType2String(type) << " ";
            left->appendString(s);
            s << " ";
            right->appendString(s);
            s << ")";
        }
    };

}
