#pragma once

#include "AssignableExpression.h"
#include "../../common/ControlType.h"

namespace MaximAst {

    class ControlExpression : public AssignableExpression {
    public:
        std::string name;
        MaximCommon::ControlType type;
        std::string prop;

        ControlExpression(std::string name, MaximCommon::ControlType type, std::string prop, SourcePos start,
                          SourcePos end)
            : AssignableExpression(start, end), name(std::move(name)), type(type), prop(prop) {}

        void appendString(std::ostream &s) override {
            s << "(control " << name << " " << MaximCommon::controlType2String(type) << " " << prop << ")";
        }
    };

}
