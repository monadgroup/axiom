#pragma once

#include <unordered_map>

#include "../common/ControlType.h"
#include "ComposableModuleClass.h"
#include "ControlField.h"

namespace MaximCodegen {

    class Type;

    class Control : public UndefInitializedModuleClass {
    public:
        Control(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, llvm::Type *storageType,
                llvm::Type *underlyingType, const std::string &name);

        MaximCommon::ControlType type() const { return _type; }

        ControlField *addField(const std::string &name, Type *type);

        ControlField *getField(const std::string &name);

        llvm::Type *storageType() override;

        llvm::Type *underlyingType() { return _underlyingType; }

        ModuleClassMethod *constructor() override { return &_constructor; }

    protected:

        void doComplete() override;

    private:
        std::unordered_map<std::string, std::unique_ptr<ControlField>> _fields;
        MaximCommon::ControlType _type;
        llvm::Type *_storageType;
        llvm::Type *_underlyingType;

        ModuleClassMethod _constructor;

    };

}
