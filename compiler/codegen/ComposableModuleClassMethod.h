#pragma once

#include "ModuleClassMethod.h"

namespace MaximCodegen {

    class ComposableModuleClass : public ModuleClass;

    class ComposableModuleClassMethod : public ModuleClassMethod {
    public:
        ComposableModuleClassMethod(ComposableModuleClass *moduleClass, std::string name, llvm::Type *returnType = nullptr, std::vector<llvm::Type*> paramTypes = {});

        ComposableModuleClass *moduleClass() const override { return _composableModuleClass; }

        llvm::Value *getEntryPointer(size_t index, const llvm::Twine &name);

        llvm::Value *callInto(size_t index, const std::vector<llvm::Value*> &args, const ModuleClassMethod *internalMethod, const llvm::Twine &resultName);

    private:
        ComposableModuleClass *_composableModuleClass;
    };

}
