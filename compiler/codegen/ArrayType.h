#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class MaximContext;

    class ArrayType : public Type {
    public:
        static constexpr size_t arraySize = 16;

        ArrayType(MaximContext *context, Type *baseType, llvm::ArrayType *type);

        llvm::ArrayType *get() const override { return _type; }

        Type *baseType() const { return _baseType; }

        std::string name() const override;

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        Type *_baseType;

        llvm::ArrayType *_type;

        MaximContext *_context;
    };

}
