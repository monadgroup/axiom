#pragma once

#include <vector>
#include <memory>
#include <sstream>

#include "Expression.h"

namespace MaximAst {

    class Expression;

    class Block {
    public:
        std::vector<std::unique_ptr<Expression>> expressions;

        void appendString(std::ostream &s) {
            for (const auto &expr : expressions) {
                expr->appendString(s);
                s << std::endl;
            }
        }
    };

}
