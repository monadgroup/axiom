#pragma once

namespace MaximAst {

    class TupleExpression : public Expression {
    public:
        std::vector<std::unique_ptr<Expression>> expressions;

        TupleExpression(SourcePos startPos, SourcePos endPos) : Expression(startPos, endPos) {}

        void appendString(std::ostream &s) override {
            s << "(tuple";
            if (!expressions.empty()) s << " ";
            for (size_t i = 0; i < expressions.size(); i++) {
                expressions[i]->appendString(s);
                if (i != expressions.size() - 1) s << " ";
            }
            s << ")";
        }
    };

}
