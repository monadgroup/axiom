#pragma once

#include <memory>
#include <unordered_map>

#include "InstantiableFunction.h"
#include "Builder.h"
#include "../common/ControlType.h"
#include "../SourcePos.h"

namespace MaximAst {
    class AssignableExpression;
    class ControlExpression;
}

namespace MaximCodegen {
    struct ControlKey {
        std::string name;
        MaximCommon::ControlType type;

        bool operator==(const MaximCodegen::ControlKey &x) const {
            return name == x.name && type == x.type;
        }
    };
}

namespace std {
    template<> struct hash<MaximCodegen::ControlKey> {
        size_t operator()(const MaximCodegen::ControlKey &x) const {
            std::hash<std::string> h;
            return h(x.name) ^ (size_t)x.type;
        }
    };
}

namespace MaximCodegen {

    class MaximContext;

    class Value;

    class Control;

    class Node : public InstantiableFunction {
    public:
        explicit Node(MaximContext *ctx, llvm::Module *module);

        std::unique_ptr<Value> getVariable(const std::string &name, SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value> getControl(Builder &b, MaximAst::ControlExpression *expr);

        void setVariable(std::string name, std::unique_ptr<Value> value);

        void setControl(Builder &b, MaximAst::ControlExpression *expr, std::unique_ptr<Value> value);

        void setAssignable(Builder &b, MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value);

    private:
        struct ControlValue {
            Control *control;
            llvm::Value *instPtr;
        };

        std::unordered_map<std::string, std::unique_ptr<Value>> _variables;
        std::unordered_map<ControlKey, ControlValue> _controls;

        ControlValue &getControl(std::string name, MaximCommon::ControlType type, Builder &b);
        std::unique_ptr<Control> createControl(MaximCommon::ControlType type);
    };

}
