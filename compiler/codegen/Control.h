#pragma once

#include <unordered_map>

#include "ComposableModuleClass.h"
#include "ControlField.h"

namespace MaximCodegen {

    class Type;

    class Control : public ModuleClass {
    public:
        Control(MaximContext *ctx, llvm::Module *module, llvm::Type *storageType, const std::string &name);

        ControlField *addField(const std::string &name, Type *type);

        llvm::Constant *initializeVal() override;

        llvm::Type *storageType() override;

        ModuleClassMethod *constructor() override { return &_constructor; }

    protected:

        void doComplete() override;

    private:
        std::unordered_map<std::string, ControlField> _fields;
        llvm::Type *_storageType;

        ModuleClassMethod _constructor;

    };

}
