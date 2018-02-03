#pragma once

#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>

#include "Value.h"

namespace MaximCodegen {

    class Function;

    class Context;

    class TupleValue : public Value {
    public:
        TupleValue(bool isConst, const std::vector<llvm::Value *> &values, Context *context, Function *function);

        TupleValue(bool isConst, llvm::Value *value, Context *context);

        llvm::StructType *type() const override { return _type; }

        llvm::Value *value() const override { return _value; }

        llvm::Value *itemPtr(unsigned int index, llvm::IRBuilder<> &builder) const;

        std::unique_ptr<Value> clone() const override;

    private:
        llvm::Value *_value;
        llvm::StructType *_type;
        Context *_context;
    };

}
