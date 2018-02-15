#include "ControlConverter.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

ControlConverter::ControlConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::CONTROL) {
    converters.emplace(MaximCommon::FormType::OSCILLATOR, (FormConverter)&MaximCodegen::ControlConverter::fromOscillator);
    converters.emplace(MaximCommon::FormType::NOTE, (FormConverter)&MaximCodegen::ControlConverter::fromNote);
    converters.emplace(MaximCommon::FormType::DB, (FormConverter)&MaximCodegen::ControlConverter::fromDb);
    converters.emplace(MaximCommon::FormType::Q, (FormConverter)&MaximCodegen::ControlConverter::fromQ);
    converters.emplace(MaximCommon::FormType::SECONDS, (FormConverter)&MaximCodegen::ControlConverter::fromSeconds);
    converters.emplace(MaximCommon::FormType::BEATS, (FormConverter)&MaximCodegen::ControlConverter::fromBeats);
}

std::unique_ptr<ControlConverter> ControlConverter::create(MaximContext *context) {
    return std::make_unique<ControlConverter>(context);
}

llvm::Value* ControlConverter::fromOscillator(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, context()->constFloat(1)));
    return b.CreateFDiv(a, llvm::ConstantVector::getSplat(2, context()->constFloat(2)));
}

llvm::Value* ControlConverter::fromNote(Builder &b, llvm::Value *val, llvm::Module *module) {
    return b.CreateFDiv(val, llvm::ConstantVector::getSplat(2, context()->constFloat(127)));
}

llvm::Value* ControlConverter::fromDb(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::pow, {context()->numType()->vecType()});
    auto d = b.CreateFDiv(val, llvm::ConstantVector::getSplat(2, context()->constFloat(20)));
    auto p = CreateCall(b, powIntrinsic, {
        llvm::ConstantVector::getSplat(2, context()->constFloat(10)),
        d
    }, "powered");
    return b.CreateFDiv(p, llvm::ConstantVector::getSplat(2, context()->constFloat(2)));
}

llvm::Value* ControlConverter::fromQ(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto m = b.CreateFMul(llvm::ConstantVector::getSplat(2, context()->constFloat(2)), val);
    auto d = b.CreateFDiv(llvm::ConstantVector::getSplat(2, context()->constFloat(1)), m);
    auto s = b.CreateFSub(llvm::ConstantVector::getSplat(2, context()->constFloat(1)), d);
    return b.CreateFDiv(s, llvm::ConstantVector::getSplat(2, context()->constFloat(0.999)));
}

llvm::Value* ControlConverter::fromSeconds(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, context()->constFloat(1.1)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, context()->constFloat(0.5)));
    return b.CreateFDiv(m, a);
}

llvm::Value* ControlConverter::fromBeats(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, context()->constFloat(1.1)));
    auto a = b.CreateFAdd(val, llvm::ConstantVector::getSplat(2, context()->constFloat(0.8)));
    return b.CreateFDiv(m, a);
}
