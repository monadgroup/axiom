#include "QConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

QConverter::QConverter(MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::Q) {
    converters.emplace(MaximCommon::FormType::CONTROL, (FormConverter) & MaximCodegen::QConverter::fromControl);
}

std::unique_ptr<QConverter> QConverter::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<QConverter>(ctx, module);
}

llvm::Value *QConverter::fromControl(ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    auto m = b.CreateFMul(ctx()->constFloatVec(-2), val);
    m = b.CreateFMul(m, ctx()->constFloatVec(0.999));
    auto a = b.CreateFAdd(m, ctx()->constFloatVec(2));
    return b.CreateFDiv(ctx()->constFloatVec(1), a);
}
