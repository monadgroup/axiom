#include "DbConverter.h"

#include "../MaximContext.h"

using namespace MaximCodegen;

DbConverter::DbConverter(MaximContext *context) : Converter(context, MaximCommon::FormType::DB) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::DbConverter::fromControl);
}

std::unique_ptr<DbConverter> DbConverter::create(MaximContext *context) {
    return std::make_unique<DbConverter>(context);
}

llvm::Value *DbConverter::fromControl(Builder &b, llvm::Value *val, llvm::Module *module) {
    auto logIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::log10,
                                                        {context()->numType()->vecType()});
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, context()->constFloat(2)));
    auto l = CreateCall(b, logIntrinsic, {m}, "logd");
    return b.CreateFMul(l, llvm::ConstantVector::getSplat(2, context()->constFloat(20)));
}
