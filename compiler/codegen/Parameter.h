#pragma once

#include <memory>

namespace llvm {
    class Type;

    class Value;
}

namespace MaximCodegen {

    class Parameter {
    public:
        Parameter(bool isConst, llvm::Type *type, llvm::Value *defaultValue = nullptr);

        bool isConst() const { return _isConst; }

        llvm::Type *type() const { return _type; }

        llvm::Value *defaultValue() const { return _defaultValue; }

    private:
        bool _isConst;
        llvm::Type *_type;
        llvm::Value *_defaultValue;
    };

}
