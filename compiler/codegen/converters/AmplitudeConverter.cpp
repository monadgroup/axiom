#include "AmplitudeConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

AmplitudeConverter::AmplitudeConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::AMPLITUDE) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::DB, std::bind(&AmplitudeConverter::fromDb, this, _1, _2));
}

std::unique_ptr<AmplitudeConverter> AmplitudeConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<AmplitudeConverter>(ctx, module);
}

llvm::Value* AmplitudeConverter::fromDb(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow, val->getType());

    auto &b = method->builder();
    return b.CreateCall(powIntrinsic, {
        ctx()->constFloatVec(10),
        b.CreateFDiv(val, ctx()->constFloatVec(20))
    });
}
