#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class MaximContext;

    class NumType : public Type {
    public:
        explicit NumType(MaximContext *context);

        llvm::StructType *get() const override { return _type; }

        llvm::VectorType *vecType() const { return _vecType; }

        llvm::Type *formType() const { return _formType; }

        llvm::Type *activeType() const { return _activeType; }

        std::string name() const override { return "num"; }

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        MaximContext *_context;

        llvm::StructType *_type;

        llvm::VectorType *_vecType;

        llvm::Type *_formType;

        llvm::Type *_activeType;
    };

}
