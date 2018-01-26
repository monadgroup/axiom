#pragma once

#include <vector>
#include <memory>

namespace MaximAst {

    class Expression;

    class Block {
    public:
        std::vector<std::unique_ptr<Expression>> expressions;
    };

}
