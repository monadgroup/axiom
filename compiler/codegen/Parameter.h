#pragma once

#include <memory>

namespace llvm {
    class Type;

    class Constant;
}

namespace MaximCodegen {

    class Parameter {
    public:
        Parameter(bool isConst, llvm::Type *type, llvm::Constant *defaultValue = nullptr);

        bool isConst() const { return _isConst; }

        llvm::Type *type() const { return _type; }

        llvm::Constant *defaultValue() const { return _defaultValue; }

    private:
        bool _isConst;
        llvm::Type *_type;
        llvm::Constant *_defaultValue;
    };

}
