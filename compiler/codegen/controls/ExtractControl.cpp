#include <llvm/Support/raw_ostream.h>
#include "ExtractControl.h"

#include "../Type.h"
#include "../MaximContext.h"

using namespace MaximCodegen;

ExtractControl::ExtractControl(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type,
                               Type *storageType, const std::string &name)
    : Control(ctx, module, type, ctx->getArrayType(storageType)->get(), storageType->get(), name + "control") {
    auto valueField = addField("value", ctx->getArrayType(storageType));

    auto getMethod = valueField->getValue();
    getMethod->builder().CreateRet(getMethod->groupPtr());

    auto setMethod = valueField->setValue();
    ctx->copyPtr(setMethod->builder(), setMethod->arg(0), setMethod->groupPtr());

    complete();
}

std::unique_ptr<ExtractControl> ExtractControl::create(MaximContext *ctx, llvm::Module *module,
                                                       MaximCommon::ControlType type, Type *storageType,
                                                       const std::string &name) {
    return std::make_unique<ExtractControl>(ctx, module, type, storageType, name);
}
