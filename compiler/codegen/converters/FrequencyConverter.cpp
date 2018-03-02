#include "FrequencyConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

FrequencyConverter::FrequencyConverter(MaximContext *ctx, llvm::Module *module) : Converter(ctx, module, MaximCommon::FormType::FREQUENCY) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::FrequencyConverter::fromControl);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter) & MaximCodegen::FrequencyConverter::fromSeconds);
    converters.emplace(MaximCommon::FormType::NOTE, (FormConverter) & MaximCodegen::FrequencyConverter::fromNote);
}

std::unique_ptr<FrequencyConverter> FrequencyConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<FrequencyConverter>(ctx, module);
}

llvm::Value *FrequencyConverter::fromControl(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow,
                                                   ctx()->numType()->vecType());

    auto &b = method->builder();
    auto baseVal = llvm::ConstantVector::getSplat(2, ctx()->constFloat(20000));
    return CreateCall(b, powFunc, {baseVal, val}, "");
}

llvm::Value *FrequencyConverter::fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto oneDiv = llvm::ConstantVector::getSplat(2, ctx()->constFloat(1));
    return method->builder().CreateFDiv(oneDiv, val);
}

llvm::Value *FrequencyConverter::fromNote(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow,
                                                   ctx()->numType()->vecType());

    auto &b = method->builder();
    auto noteSub = b.CreateFSub(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(69)));
    auto noteDiv = b.CreateFDiv(noteSub, llvm::ConstantVector::getSplat(2, ctx()->constFloat(12)));
    auto powd = CreateCall(b, powFunc, {
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)),
        noteDiv
    }, "");
    return b.CreateFMul(llvm::ConstantVector::getSplat(2, ctx()->constFloat(440)), powd);
}
