#pragma once

#include <llvm/IR/DerivedTypes.h>
#include <vector>
#include <memory>

#include "Type.h"

namespace MaximCodegen {

    class TupleType : public Type {
    public:
        TupleType(Context *context, std::vector<std::unique_ptr<Type>> types);

        llvm::StructType *llType() const override { return _llType; }

        std::vector<std::unique_ptr<Type>> const &types() const { return _types; }

    private:
        llvm::StructType *_llType;
        std::vector<std::unique_ptr<Type>> _types;
    };

}
