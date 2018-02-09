#pragma once

#include "../Context.h"
#include "../../ast/Form.h"
#include "../Builder.h"

namespace MaximCodegen {

    class Function;

    class FormValue {
    public:
        using ParamArr = std::array<llvm::Value *, Context::formParamCount>;

        FormValue(MaximAst::Form::Type formType, const ParamArr &params, Context *context, Function *function);

        FormValue(llvm::Value *formType, llvm::Value *params, Context *context, Function *function);

        FormValue(llvm::Value *value, Context *context);

        llvm::Type *type() const;

        llvm::Value *value() const { return _value; }

        llvm::Value *typePtr(Builder &builder) const;

        llvm::Value *paramsPtr(Builder &builder) const;

    private:
        Context *_context;
        llvm::Value *_value;
    };

}
