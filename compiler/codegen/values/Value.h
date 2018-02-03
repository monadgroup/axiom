#pragma once

#include <memory>

namespace llvm {
    class Type;

    class Value;
}

namespace MaximCodegen {

    class Value {
    public:
        explicit Value(bool isConst);

        virtual llvm::Type *type() const = 0;

        virtual llvm::Value *value() const = 0;

        bool isConst() const { return _isConst; }

        virtual std::unique_ptr<Value> clone() const = 0;

    private:
        bool _isConst;
    };

}
