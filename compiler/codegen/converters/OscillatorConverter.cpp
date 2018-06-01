#include "OscillatorConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

OscillatorConverter::OscillatorConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::OSCILLATOR) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&OscillatorConverter::fromControl, this, _1, _2));
}

std::unique_ptr<OscillatorConverter> OscillatorConverter::create(MaximCodegen::MaximContext *ctx,
                                                                 llvm::Module *module) {
    return std::make_unique<OscillatorConverter>(ctx, module);
}

llvm::Value* OscillatorConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFSub(
        b.CreateFMul(val, ctx()->constFloatVec(2)),
        ctx()->constFloatVec(1)
    );
}
