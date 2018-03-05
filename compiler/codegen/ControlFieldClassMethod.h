#pragma once

#include "ModuleClassMethod.h"

namespace MaximCodegen {

    class ControlField;

    class ControlFieldClassMethod : public ModuleClassMethod {
    public:
        ControlFieldClassMethod(ControlField *controlField, std::string name, llvm::Type *returnType = nullptr,
                                std::vector<llvm::Type *> paramTypes = {});

        llvm::Value *groupPtr() const { return _groupPtr; }

    private:
        llvm::Value *_groupPtr;
    };

}
