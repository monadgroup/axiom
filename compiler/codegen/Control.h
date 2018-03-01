#pragma once

#include <unordered_map>

#include "../common/ControlType.h"
#include "ComposableModuleClass.h"
#include "ControlField.h"

namespace MaximCodegen {

    class Type;

    class Control : public ModuleClass {
    public:
        Control(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, llvm::Type *storageType, const std::string &name);

        MaximCommon::ControlType type() const { return _type; }

        ControlField *addField(const std::string &name, Type *type);

        ControlField *getField(const std::string &name) const;

        llvm::Constant *initializeVal() override;

        llvm::Type *storageType() override;

        ModuleClassMethod *constructor() override { return &_constructor; }

    protected:

        void doComplete() override;

    private:
        std::unordered_map<std::string, ControlField> _fields;
        MaximCommon::ControlType _type;
        llvm::Type *_storageType;

        ModuleClassMethod _constructor;

    };

}
