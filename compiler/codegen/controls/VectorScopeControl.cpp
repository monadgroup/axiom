#include "VectorScopeControl.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

VectorScopeControl::VectorScopeControl(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Control(ctx, module, MaximCommon::ControlType::SCOPE, llvm::StructType::get(ctx->llvm(), {
        llvm::Type::getInt16Ty(ctx->llvm()), // buffer position
        llvm::Type::getInt16Ty(ctx->llvm()), // buffer capacity
        llvm::ArrayType::get(ctx->floatVecTy(), ctx->sampleRate / minimumFps)
    }, false), ctx->numType()->get(), ctx->numType()->get(), "vectorcontrol") {

    auto valueField = addField("value", ctx->numType());

    auto getMethod = valueField->getValue();
    getMethod->builder().CreateRet(getMethod->groupPtr());

    auto setMethod = valueField->setValue();
    ctx->copyPtr(setMethod->builder(), setMethod->arg(0), setMethod->groupPtr());

    complete();
}

std::unique_ptr<VectorScopeControl> VectorScopeControl::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<VectorScopeControl>(ctx, module);
}
