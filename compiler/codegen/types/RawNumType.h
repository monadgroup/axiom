#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class RawNumType : public Type {
    public:
        explicit RawNumType(Context *context);

        llvm::StructType *llType() const override { return _llType; }
        llvm::Type *leftType() const { return _leftType; }
        llvm::Type *rightType() const { return _rightType; }

    private:
        llvm::StructType *_llType;
        llvm::Type *_leftType;
        llvm::Type *_rightType;
    };

}
