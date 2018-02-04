#pragma once

#include <memory>
#include <unordered_map>

#include "../ast/ControlExpression.h"

namespace MaximCodegen {
    struct ControlKey {
        std::string name;
        MaximAst::ControlExpression::Type type;

        bool operator==(const ControlKey &other) const;
    };
}

namespace std {
    template<> struct hash<MaximCodegen::ControlKey> {
        size_t operator()(const MaximCodegen::ControlKey &x) const {
            std::hash<std::string> h;
            return h(x.name) ^ (size_t) x.type;
        }
    };
}

namespace MaximCodegen {

    class Value;

    class Control;

    class Context;

    class Scope {
    public:
        explicit Scope(Context *context);

        Value *findValue(std::string name);

        void setValue(std::string name, std::unique_ptr<Value> value);

        Control *getControl(std::string name, MaximAst::ControlExpression::Type type);

    private:
        Context *_context;

        std::unordered_map<std::string, std::unique_ptr<Value>> values;
        std::unordered_map<ControlKey, std::unique_ptr<Control>> controls;
    };

}
