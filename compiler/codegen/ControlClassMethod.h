#pragma once

#include "ModuleClassMethod.h"

namespace MaximCodegen {

    class ControlField;

    class ControlClassMethod : public ModuleClassMethod {
    public:
        ControlClassMethod(ModuleClass *parentClass, std::string name, llvm::Type *returnType = nullptr,
                                std::vector<llvm::Type *> paramTypes = {});

        llvm::Value *groupPtr() const { return _groupPtr; }

        llvm::Value *storagePtr() const { return _storagePtr; }

    private:
        llvm::Value *_groupPtr;
        llvm::Value *_storagePtr;
    };

}
