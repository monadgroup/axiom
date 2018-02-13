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

        Tuple(MaximContext *context, Storage values, Builder &builder, SourcePos startPos, SourcePos endPos);

        Tuple(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Tuple>
        create(MaximContext *context, Storage values, Builder &builder, SourcePos startPos, SourcePos endPos);

        static std::unique_ptr<Tuple>
        create(MaximContext *context, TupleType *type, llvm::Value *get, SourcePos startPos, SourcePos endPos);

        std::unique_ptr<Value> atIndex(size_t index, Builder &builder, SourcePos startPos, SourcePos endPos) const;

        llvm::Value *get() const override { return _get; }

        std::unique_ptr<Value> withSource(SourcePos startPos, SourcePos endPos) const override;

        std::unique_ptr<Tuple> withIndex(size_t index, std::unique_ptr<Value> val, Builder &builder, SourcePos startPos,
                                         SourcePos endPos) const;

        TupleType *type() const override { return _type; }

    private:
        TupleType *_type;
        llvm::Value *_get;
        MaximContext *_context;
    };

}
