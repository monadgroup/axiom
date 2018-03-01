#pragma once

#include <memory>
#include <unordered_map>

#include "../common/ControlType.h"
#include "../SourcePos.h"

namespace llvm {
    class Value;
}

namespace MaximAst {
    class ControlExpression;

    class AssignableExpression;
}

namespace MaximCodegen {
    class Control;

    struct ControlKey {
        std::string name;
        MaximCommon::ControlType type;

        bool operator==(const MaximCodegen::ControlKey &x) const {
            return name == x.name && type == x.type;
        }
    };

    struct ControlInstance {
        Control *control;
        bool isWrittenTo;
        uint64_t instId;
    };
}

namespace std {
    template<>
    struct hash<MaximCodegen::ControlKey> {
        size_t operator()(const MaximCodegen::ControlKey &x) const {
            std::hash<std::string> h;
            return h(x.name) ^ (size_t) x.type;
        }
    };
}

namespace MaximCodegen {

    class Value;

    class ComposableModuleClassMethod;

    class Scope {
    public:
        std::unique_ptr<Value> getVariable(const std::string &name, SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value> getControl(ComposableModuleClassMethod *method, MaximAst::ControlExpression *expr);

        void setVariable(std::string name, std::unique_ptr<Value> value);

        void setControl(ComposableModuleClassMethod *method, MaximAst::ControlExpression *expr, std::unique_ptr<Value> value);

        void setAssignable(ComposableModuleClassMethod *method, MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value);

    private:
        std::unordered_map<std::string, std::unique_ptr<Value>> _variables;
        std::unordered_map<ControlKey, ControlInstance> _controls;

        ControlInstance &getControl(const std::string &name, MaximCommon::ControlType type, ComposableModuleClassMethod *method);
    };

}
