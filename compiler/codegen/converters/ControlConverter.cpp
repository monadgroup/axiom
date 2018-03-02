#include "ControlConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

ControlConverter::ControlConverter(MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::CONTROL) {
    converters.emplace(MaximCommon::FormType::OSCILLATOR,
                       (FormConverter) & MaximCodegen::ControlConverter::fromOscillator);
    converters.emplace(MaximCommon::FormType::FREQUENCY,
                       (FormConverter) & MaximCodegen::ControlConverter::fromFrequency);
    converters.emplace(MaximCommon::FormType::NOTE, (FormConverter) & MaximCodegen::ControlConverter::fromNote);
    converters.emplace(MaximCommon::FormType::DB, (FormConverter) & MaximCodegen::ControlConverter::fromDb);
    converters.emplace(MaximCommon::FormType::Q, (FormConverter) & MaximCodegen::ControlConverter::fromQ);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter) & MaximCodegen::ControlConverter::fromSeconds);
    converters.emplace(MaximCommon::FormType::BEATS, (FormConverter) & MaximCodegen::ControlConverter::fromBeats);
}

std::unique_ptr<ControlConverter> ControlConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ControlConverter>(ctx, module);
}

llvm::Value *ControlConverter::fromOscillator(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)));
    return b.CreateFDiv(a, llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)));
}

llvm::Value *ControlConverter::fromFrequency(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto logFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::log,
                                                   ctx()->numType()->vecType());

    auto &b = method->builder();
    auto baseVal = llvm::ConstantVector::getSplat(2, ctx()->constFloat(20000));
    auto logBase = CreateCall(b, logFunc, {baseVal}, "logbase");
    auto logInput = CreateCall(b, logFunc, {val}, "loginput");

    return b.CreateFDiv(logInput, logBase);
}

llvm::Value *ControlConverter::fromNote(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(127)));
}

llvm::Value *ControlConverter::fromDb(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow,
                                                        {ctx()->numType()->vecType()});
    auto &b = method->builder();
    auto d = b.CreateFDiv(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(20)));
    auto p = CreateCall(b, powIntrinsic, {
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(10)),
        d
    }, "powered");
    return b.CreateFDiv(p, llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)));
}

llvm::Value *ControlConverter::fromQ(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)), val);
    auto d = b.CreateFDiv(llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)), m);
    auto s = b.CreateFSub(llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)), d);
    return b.CreateFDiv(s, llvm::ConstantVector::getSplat(2, ctx()->constFloat(0.999)));
}

llvm::Value *ControlConverter::fromSeconds(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1.1)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(0.5)));
    return b.CreateFDiv(m, a);
}

llvm::Value *ControlConverter::fromBeats(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1.1)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(0.8)));
    return b.CreateFDiv(m, a);
}
