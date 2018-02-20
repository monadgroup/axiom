#pragma once

#include <vector>
#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace llvm {
    class StructLayout;
}

namespace MaximCodegen {

    class MaximContext;

    class TupleType : public Type {
    public:
        TupleType(MaximContext *context, std::vector<Type *> types, llvm::StructType *type);

        llvm::StructType *get() const override { return _type; }

        std::vector<Type *> const &types() const { return _types; }

        std::string name() const override;

        std::unique_ptr<Value> createInstance(llvm::Value *val, SourcePos startPos, SourcePos endPos) override;

        const llvm::StructLayout *layout() const { return _layout; }

    private:
        std::vector<Type *> _types;

        llvm::StructType *_type;

        MaximContext *_context;

        const llvm::StructLayout *_layout;
    };

}
