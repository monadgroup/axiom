#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class MaximContext;

    class ArrayType : public Type {
    public:
        static constexpr size_t arraySize = 32;

        ArrayType(MaximContext *context, Type *baseType);

        llvm::ArrayType *get() const override { return _type; }

        llvm::IntegerType *itemEnabledType() const { return _itemEnabledType; }

        llvm::Type *itemValType() const { return _itemValType; }

        llvm::StructType *itemType() const { return _itemType; }

        Type *baseType() const { return _baseType; }

        std::string name() const override;

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        Type *_baseType;

        llvm::ArrayType *_type;

        llvm::IntegerType *_itemEnabledType;

        llvm::Type *_itemValType;

        llvm::StructType *_itemType;

        MaximContext *_context;
    };

}
