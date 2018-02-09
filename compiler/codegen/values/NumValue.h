#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>

#include "Value.h"
#include "../Context.h"
#include "../../ast/Form.h"
#include "FormValue.h"

namespace MaximCodegen {

    class Function;

    class NumValue : public Value {
    public:
        using ParamArr = std::array<llvm::Value *, Context::formParamCount>;

        NumValue(bool isConst, llvm::Value *value, const FormValue &form, Context *context, Function *function);

        NumValue(bool isConst, llvm::Value *value, Context *context);

        static std::unique_ptr<NumValue> fromRegister(bool isConst, llvm::Value *value, Context *context, Function *function);

        llvm::StructType *type() const override;

        llvm::Value *value() const override { return _value; }

        llvm::Value *valuePtr(Builder &builder) const;

        llvm::Value *formPtr(Builder &builder) const;

        std::unique_ptr<Value> clone() const override;

    private:
        llvm::Value *_value;
        Context *_context;
    };

}
