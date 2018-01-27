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
                : AssignableExpression(start, end), name(std::move(name)), type(type), prop(prop) {}

        void appendString(std::stringstream &s) override {
            s << "(control " << name << " ";
            switch (type) {
                case Type::LABEL:
                    s << "label";
                    break;
                case Type::VALUE:
                    s << "value";
                    break;
                case Type::TOGGLE:
                    s << "toggle";
                    break;
                case Type::GRAPH:
                    s << "graph";
                    break;
                case Type::SCOPE:
                    s << "scope";
                    break;
                case Type::KEYS:
                    s << "keys";
                    break;
                case Type::ROLL:
                    s << "roll";
                    break;
                case Type::PLUG:
                    s << "plug";
                    break;
            }
            s << " " << prop << ")";
        }
    };

}
