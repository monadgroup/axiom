#pragma once

#include <string>
#include <unordered_map>

#include "Context.h"
#include "../ast/ControlExpression.h"

namespace MaximCodegen {

    class ControlDeclaration {
    public:
        using ParamsMap = std::unordered_map<std::string, Context::Type>;

        explicit ControlDeclaration(MaximAst::ControlExpression::Type type);

        MaximAst::ControlExpression::Type type() const { return _type; }
        std::unordered_map<std::string, Context::Type> &params() { return _params; };

    private:
        MaximAst::ControlExpression::Type _type;
        std::unordered_map<std::string, Context::Type> _params;
    };

}
