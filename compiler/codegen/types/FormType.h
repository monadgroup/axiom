#pragma once

#include <llvm/IR/DerivedTypes.h>

#include "Type.h"

namespace MaximCodegen {

    class Form;

    class FormType : public Type {
    public:
        FormType(Context *context, Form *form);

        llvm::StructType *llType() const override { return _llType; }
        llvm::Type *typeType() const { return _typeType; }
        llvm::Type *parametersType() const { return _parametersType; }

        Form *form() const { return _form; }

    private:
        llvm::StructType *_llType;
        llvm::Type *_typeType;
        llvm::Type *_parametersType;
        Form *_form;
    };

}
