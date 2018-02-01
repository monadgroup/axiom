#pragma once

#include <memory>

namespace MaximCodegen {

    class Type;

    class Value;

    class Parameter {
    public:
        Parameter(bool isConst, std::unique_ptr<Type> type, std::unique_ptr<Value> defaultValue);

        bool isConst() const { return _isConst; }

        Type *type() const { return _type.get(); }

        Value *defaultValue() const { return _defaultValue.get(); }

    private:
        bool _isConst;
        std::unique_ptr<Type> _type;
        std::unique_ptr<Value> _defaultValue;
    };

}
