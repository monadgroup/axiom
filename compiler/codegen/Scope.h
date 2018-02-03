#pragma once

#include <memory>

#include "../ast/ControlExpression.h"

namespace MaximCodegen {

    class Value;

    class Scope {
    public:
        Value *findValue(std::string name);
        void setValue(std::string name, std::unique_ptr<Value> value);

        Value *findControl(std::string name, MaximAst::ControlExpression::Type type, std::string property);
        void setControl(std::string name, MaximAst::ControlExpression::Type type, std::string property, std::unique_ptr<Value> value);
    };

}
