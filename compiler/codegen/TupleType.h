#pragma once

namespace llvm {
    class Type;

    class StructType;
}

namespace MaximCodegen {

    class TupleType {
    public:
        TupleType(std::vector<llvm::Type *> types);

        llvm::StructType *type() const;

        std::vector<llvm::Type *> const &types() const;

    private:
        std::vector<llvm::Type *> _types;
    };

}
