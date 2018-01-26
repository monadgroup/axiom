#pragma once

#include "AssignableExpression.h"

namespace MaximAst {

    class ControlExpression : public AssignableExpression {
    public:
        enum class Type {
            LABEL,
            VALUE,
            TOGGLE,
            GRAPH,
            SCOPE,
            KEYS,
            ROLL,
            PLUG
        };

        std::string name;
        Type type;
        std::string prop;

        ControlExpression(std::string name, Type type, std::string prop, SourcePos start, SourcePos end)
                : AssignableExpression(start, end), name(std::move(name)), type(type), prop(prop) { }
    };

}
