#include "FrequencyConverter.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

FrequencyConverter::FrequencyConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::FREQUENCY) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter)&MaximCodegen::FrequencyConverter::fromControl);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter)&MaximCodegen::FrequencyConverter::fromSeconds);
    converters.emplace(MaximCommon::FormType::NOTE, (FormConverter)&MaximCodegen::FrequencyConverter::fromNote);
}

std::unique_ptr<FrequencyConverter> FrequencyConverter::create(MaximContext *context) {
    return std::make_unique<FrequencyConverter>(context);
}

llvm::Value* FrequencyConverter::fromControl(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto powFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::pow, context()->numType()->vecType());

    auto baseVal = llvm::ConstantVector::getSplat(2, context()->constFloat(20000));
    return CreateCall(b, powFunc, {baseVal, val}, "");
}

llvm::Value* FrequencyConverter::fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto oneDiv = llvm::ConstantVector::getSplat(2, context()->constFloat(1));
    return b.CreateFDiv(oneDiv, val);
}

llvm::Value* FrequencyConverter::fromNote(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto powFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::pow, context()->numType()->vecType());

    auto noteSub = b.CreateFSub(val, llvm::ConstantVector::getSplat(2, context()->constFloat(69)));
    auto noteDiv = b.CreateFDiv(noteSub, llvm::ConstantVector::getSplat(2, context()->constFloat(12)));
    auto powd = CreateCall(b, powFunc, {
        llvm::ConstantVector::getSplat(2, context()->constFloat(2)),
        noteDiv
    }, "");
    return b.CreateFMul(llvm::ConstantVector::getSplat(2, context()->constFloat(440)), powd);
}
