#include "BeatsConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

BeatsConverter::BeatsConverter(MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::BEATS) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::BeatsConverter::fromControl);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter) & MaximCodegen::BeatsConverter::fromSeconds);
}

std::unique_ptr<BeatsConverter> BeatsConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<BeatsConverter>(ctx, module);
}

llvm::Value *BeatsConverter::fromControl(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(0.8)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1.1)));
    return b.CreateFDiv(m, a);
}

llvm::Value *BeatsConverter::fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFMul(val, b.CreateLoad(ctx()->beatsPerSecond(), "bps"), "beats");
}
