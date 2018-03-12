#pragma once

#include <memory>
#include <vector>

#include "Value.h"
#include "TupleType.h"
#include "Builder.h"

namespace llvm {
    class Value;
}

namespace MaximCodegen {

    class MaximContext;

    class Tuple : public Value {
    public:
        using Storage = std::vector<std::unique_ptr<Value>>;

        Tuple(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        Tuple(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Tuple>
        create(MaximContext *context, Storage values, Builder &builder, Builder &allocaBuilder, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Tuple>
        create(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        llvm::Value *get() const { return _get; }

        llvm::Value *indexPtr(size_t index, Builder &builder) const;

        void setIndex(size_t index, Value *val, Builder &builder) const;

        std::unique_ptr<Value> atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) const;

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        TupleType *type() const override { return _type; }

    private:
        TupleType *_type;
        llvm::Value *_get;
        MaximContext *_context;
    };

}
