#pragma once

#include "Value.h"
#include "ArrayType.h"
#include "Builder.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class MaximContext;

    class Array : public Value {
    public:
        using Storage = std::vector<std::unique_ptr<Value>>;

        Array(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        Array(MaximContext *context, ArrayType *type, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Array>
        create(MaximContext *context, ArrayType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Array>
        create(MaximContext *context, ArrayType *type, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        llvm::Value *indexPtr(size_t index, Builder &builder);

        llvm::Value *indexPtr(llvm::Value *index, Builder &builder);

        std::unique_ptr<Value> atIndex(size_t index, Builder &builder);

        std::unique_ptr<Value> atIndex(llvm::Value *index, Builder &builder);

        llvm::Value *get() const override { return _get; }

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        ArrayType *type() const override { return _type; }

    private:
        ArrayType *_type;
        llvm::Value *_get;
        MaximContext *_context;
    };

}
