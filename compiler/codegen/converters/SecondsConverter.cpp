#include "SecondsConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

SecondsConverter::SecondsConverter(MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::SECONDS) {
    converters.emplace(MaximCommon::FormType::BEATS, (FormConverter) & MaximCodegen::SecondsConverter::fromBeats);
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::SecondsConverter::fromControl);
    converters.emplace(MaximCommon::FormType::FREQUENCY,
                       (FormConverter) & MaximCodegen::SecondsConverter::fromFrequency);
}

std::unique_ptr<SecondsConverter> SecondsConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SecondsConverter>(ctx, module);
}

llvm::Value *SecondsConverter::fromBeats(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(val, b.CreateLoad(ctx()->beatsPerSecond(), "bps"), "secs");
}

llvm::Value *SecondsConverter::fromControl(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(0.5)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1.1)));
    return b.CreateFDiv(m, a);
}

llvm::Value *SecondsConverter::fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto oneDiv = llvm::ConstantVector::getSplat(2, ctx()->constFloat(1));
    return method->builder().CreateFDiv(oneDiv, val);
}
