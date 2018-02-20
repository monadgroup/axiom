#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace llvm {
    class StructLayout;
}

namespace MaximCodegen {

    class MaximContext;

    class NumType : public Type {
    public:
        explicit NumType(MaximContext *context);

        llvm::StructType *get() const override { return _type; }

        llvm::VectorType *vecType() const { return _vecType; }

        llvm::IntegerType *formType() const { return _formType; }

        llvm::IntegerType *activeType() const { return _activeType; }

        const llvm::StructLayout *layout() const { return _layout; }

        std::string name() const override { return "num"; }

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

    private:
        MaximContext *_context;

        llvm::StructType *_type;

        llvm::VectorType *_vecType;

        llvm::IntegerType *_formType;

        llvm::IntegerType *_activeType;

        const llvm::StructLayout *_layout;
    };

}
