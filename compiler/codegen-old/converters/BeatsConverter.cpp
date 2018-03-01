#include "BeatsConverter.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

BeatsConverter::BeatsConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::BEATS) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::BeatsConverter::fromControl);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter) & MaximCodegen::BeatsConverter::fromSeconds);
}

std::unique_ptr<BeatsConverter> BeatsConverter::create(MaximContext *context) {
    return std::make_unique<BeatsConverter>(context);
}

llvm::Value *BeatsConverter::fromControl(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, context()->constFloat(0.8)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, context()->constFloat(1.1)));
    return b.CreateFDiv(m, a);
}

llvm::Value *BeatsConverter::fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module) {
    return b.CreateFMul(val, b.CreateLoad(context()->beatsPerSecond(), "bps"), "beats");
}
