#include "DbConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

DbConverter::DbConverter(MaximContext *ctx, llvm::Module *module) : Converter(ctx, module, MaximCommon::FormType::DB) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::DbConverter::fromControl);
}

std::unique_ptr<DbConverter> DbConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<DbConverter>(ctx, module);
}

llvm::Value *DbConverter::fromControl(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto logIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::log10,
                                                        {ctx()->numType()->vecType()});
    auto &b = method->builder();
    auto m = b.CreateFMul(val, llvm::ConstantVector::getSplat(2, ctx()->constFloat(2)));
    auto l = CreateCall(b, logIntrinsic, {m}, "logd");
    return b.CreateFMul(l, llvm::ConstantVector::getSplat(2, ctx()->constFloat(20)));
}
