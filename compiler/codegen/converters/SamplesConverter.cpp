#include "SamplesConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

SamplesConverter::SamplesConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::SAMPLES) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::BEATS, std::bind(&SamplesConverter::fromBeats, this, _1, _2));
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&SamplesConverter::fromControl, this, _1, _2));
    converters.emplace(MaximCommon::FormType::FREQUENCY, std::bind(&SamplesConverter::fromFrequency, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SECONDS, std::bind(&SamplesConverter::fromSeconds, this, _1, _2));
}

std::unique_ptr<SamplesConverter> SamplesConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SamplesConverter>(ctx, module);
}

llvm::Value* SamplesConverter::fromBeats(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFMul(val, ctx()->constFloatVec(60 * ctx()->sampleRate)),
        b.CreateLoad(ctx()->beatsPerSecondPtr(*method->moduleClass()->module()))
    );
}

llvm::Value* SamplesConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFMul(val, ctx()->constFloatVec(ctx()->sampleRate)),
        b.CreateFSub(
            ctx()->constFloatVec(11),
            b.CreateFMul(val, ctx()->constFloatVec(10))
        )
    );
}

llvm::Value* SamplesConverter::fromFrequency(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(ctx()->constFloatVec(ctx()->sampleRate), val);
}

llvm::Value* SamplesConverter::fromSeconds(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFMul(val, ctx()->constFloatVec(ctx()->sampleRate));
}
