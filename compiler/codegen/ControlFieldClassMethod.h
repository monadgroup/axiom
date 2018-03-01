#pragma once

#include "ModuleClassMethod.h"

namespace MaximCodegen {

    class ControlField;

    class ControlFieldClassMethod : public ModuleClassMethod {
    public:
        ControlFieldClassMethod(ControlField *controlField, std::string name, llvm::Type *returnType = nullptr, std::vector<llvm::Type*> paramTypes = {});

        llvm::Value *contextPtr() const override { return _contextLoadedPtr; }

    private:
        llvm::Value *_contextLoadedPtr;
    };

}
