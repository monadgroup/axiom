#pragma once

#include <llvm/IR/DerivedTypes.h>
#include <memory>

#include "Type.h"

namespace MaximCodegen {

    class RawNumType;
    class FormType;
    class Form;

    class NumType : public Type {
    public:
        NumType(Context *context, Form *form);

        llvm::StructType *llType() const override { return _llType; }
        RawNumType *numType() const { return _numType.get(); }
        FormType *formType() const { return _formType.get(); }

    private:
        llvm::StructType *_llType;
        std::unique_ptr<RawNumType> _numType;
        std::unique_ptr<FormType> _formType;
    };

}
