#include "DbConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

DbConverter::DbConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::DB) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::AMPLITUDE, std::bind(&DbConverter::fromAmplitude, this, _1, _2));
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&DbConverter::fromControl, this, _1, _2));
}

std::unique_ptr<DbConverter> DbConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<DbConverter>(ctx, module);
}

llvm::Value* DbConverter::fromAmplitude(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto log10Intrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::log10, val->getType());

    auto &b = method->builder();
    return b.CreateFMul(
        b.CreateCall(log10Intrinsic, {val}),
        ctx()->constFloatVec(20)
    );
}

llvm::Value* DbConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto log10Intrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::log10, val->getType());

    auto &b = method->builder();
    return b.CreateFMul(
        b.CreateCall(log10Intrinsic, {
            b.CreateFMul(val, ctx()->constFloatVec(2))
        }),
        ctx()->constFloatVec(20)
    );
}
