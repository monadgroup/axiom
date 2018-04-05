#pragma once

#include "ModuleClass.h"

namespace MaximCodegen {

    class ComposableModuleClassMethod;

    class ComposableModuleClass : public ModuleClass {
    public:
        ComposableModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name,
                              const std::vector<llvm::Type *> &constructorParams = {});

        ModuleClassMethod *constructor() override;

        ComposableModuleClassMethod *cconstructor() const { return _constructor.get(); }

        std::unique_ptr<ComposableModuleClassMethod> entryAccessor(size_t index);

        llvm::StructType *storageType() override;

        size_t addEntry(llvm::Type *type);

        size_t addEntry(ModuleClass *moduleClass, const std::vector<llvm::Value *> &constructorParams = {}, bool callConstructor = true);

        llvm::Value *getEntryPointer(Builder &b, size_t index, llvm::Value *context, const llvm::Twine &resultName);

    protected:

        void doComplete() override;

    private:
        std::vector<llvm::Type *> _typeDict;
        std::vector<ModuleClass *> _moduleClasses;

        std::unique_ptr<ComposableModuleClassMethod> _constructor;
    };

}
