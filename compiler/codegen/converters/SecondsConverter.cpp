#include "SecondsConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

SecondsConverter::SecondsConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::SECONDS) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::BEATS, std::bind(&SecondsConverter::fromBeats, this, _1, _2));
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&SecondsConverter::fromControl, this, _1, _2));
    converters.emplace(MaximCommon::FormType::FREQUENCY, std::bind(&SecondsConverter::fromFrequency, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SAMPLES, std::bind(&SecondsConverter::fromSamples, this, _1, _2));
}

std::unique_ptr<SecondsConverter> SecondsConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SecondsConverter>(ctx, module);
}

llvm::Value* SecondsConverter::fromBeats(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        val,
        b.CreateFDiv(b.CreateLoad(ctx()->beatsPerSecondPtr(*method->moduleClass()->module())), ctx()->constFloatVec(60))
    );
}

llvm::Value* SecondsConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        val,
        b.CreateFSub(
            ctx()->constFloatVec(2.2),
            b.CreateFMul(val, ctx()->constFloatVec(2))
        )
    );
}

llvm::Value* SecondsConverter::fromFrequency(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(ctx()->constFloatVec(1), val);
}

llvm::Value* SecondsConverter::fromSamples(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(val, ctx()->constFloatVec(ctx()->sampleRate));
}
