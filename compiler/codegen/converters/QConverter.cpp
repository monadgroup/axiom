#include "QConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

QConverter::QConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::Q) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::Q, std::bind(&QConverter::fromControl, this, _1, _2));
}

std::unique_ptr<QConverter> QConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<QConverter>(ctx, module);
}

llvm::Value* QConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *value) {
    auto &b = method->builder();
    return b.CreateFDiv(
        ctx()->constFloatVec(-0.5f),
        b.CreateFSub(
            b.CreateFMul(value, ctx()->constFloatVec(0.999f)),
            ctx()->constFloatVec(1)
        )
    );
}
