#include "ScalarControl.h"

#include "../Type.h"
#include "../MaximContext.h"

using namespace MaximCodegen;

ScalarControl::ScalarControl(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType,
                             const std::string &name)
    : Control(ctx, module, type, llvm::StructType::get(ctx->llvm(), {}, false), storageType->get(), storageType->get(), name + "control") {
    auto valueField = addField("value", storageType);

    auto getMethod = valueField->getValue();
    getMethod->builder().CreateRet(getMethod->groupPtr());

    auto setMethod = valueField->setValue();
    ctx->copyPtr(setMethod->builder(), setMethod->arg(0), setMethod->groupPtr());

    complete();
}

std::unique_ptr<ScalarControl> ScalarControl::create(MaximContext *ctx, llvm::Module *module,
                                                     MaximCommon::ControlType type, Type *storageType,
                                                     const std::string &name) {
    return std::make_unique<ScalarControl>(ctx, module, type, storageType, name);
}
