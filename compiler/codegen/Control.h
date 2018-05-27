#pragma once

#include <unordered_map>

#include "../common/ControlType.h"
#include "ComposableModuleClass.h"
#include "ControlField.h"

namespace MaximCodegen {

    class Type;

    class Control : public ModuleClass {
    public:
        Control(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, llvm::Type *dataType,
                llvm::Type *valueType, llvm::Type *underlyingType, const std::string &name);

        MaximCommon::ControlType type() const { return _type; }

        ControlField *addField(const std::string &name, Type *type);

        ControlField *getField(const std::string &name);

        llvm::Type *storageType() override;

        llvm::Type *dataType() { return _dataType; }

        llvm::Type *valueType() { return _valueType; }

        llvm::Type *underlyingType() { return _underlyingType; }

        ControlClassMethod *constructor() override { return &_constructor; }

        ControlClassMethod *update() { return &_update; }

    protected:

        void doComplete() override;

    private:
        std::unordered_map<std::string, std::unique_ptr<ControlField>> _fields;
        MaximCommon::ControlType _type;
        llvm::Type *_dataType;
        llvm::Type *_valueType;
        llvm::Type *_underlyingType;

        ControlClassMethod _constructor;
        ControlClassMethod _update;

    };

}
