#include "SecondsConverter.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

SecondsConverter::SecondsConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::SECONDS) {
    converters.emplace(MaximCommon::FormType::BEATS, (FormConverter) & MaximCodegen::SecondsConverter::fromBeats);
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::SecondsConverter::fromControl);
    converters.emplace(MaximCommon::FormType::FREQUENCY,
                       (FormConverter) & MaximCodegen::SecondsConverter::fromFrequency);
}

std::unique_ptr<SecondsConverter> SecondsConverter::create(MaximContext *context) {
    return std::make_unique<SecondsConverter>(context);
}

llvm::Value *SecondsConverter::fromBeats(Builder &b, llvm::Value *val, llvm::Module *module) {
    return b.CreateFDiv(val, b.CreateLoad(context()->beatsPerSecond(), "bps"), "secs");
}

llvm::Value *SecondsConverter::fromControl(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, context()->constFloat(0.5)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, context()->constFloat(1.1)));
    return b.CreateFDiv(m, a);
}

llvm::Value *SecondsConverter::fromFrequency(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto oneDiv = llvm::ConstantVector::getSplat(2, context()->constFloat(1));
    return b.CreateFDiv(oneDiv, val);
}
