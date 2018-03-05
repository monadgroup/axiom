#include "ScalarControl.h"

#include "../Type.h"

using namespace MaximCodegen;

ScalarControl::ScalarControl(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType,
                             const std::string &name)
    : Control(ctx, module, type, storageType->get(), name + "control") {
    auto valueField = addField("value", storageType);

    auto getMethod = valueField->getValue();
    getMethod->builder().CreateRet(getMethod->builder().CreateLoad(getMethod->groupPtr(), "value"));

    auto setMethod = valueField->setValue();
    setMethod->builder().CreateStore(setMethod->arg(0), setMethod->groupPtr());

    complete();
}

std::unique_ptr<ScalarControl> ScalarControl::create(MaximContext *ctx, llvm::Module *module,
                                                     MaximCommon::ControlType type, Type *storageType,
                                                     const std::string &name) {
    return std::make_unique<ScalarControl>(ctx, module, type, storageType, name);
}
