#pragma once

#include <utility>
#include <vector>
#include <memory>

#include "Expression.h"

namespace MaximAst {

    class CallExpression : public Expression {
    public:
        std::string name;
        std::vector<std::unique_ptr<Expression>> arguments;

        CallExpression(std::string name, SourcePos start, SourcePos end)
                : Expression(start, end), name(std::move(name)) { }
    };

}
