#include "FrequencyConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

FrequencyConverter::FrequencyConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::FREQUENCY) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::BEATS, std::bind(&FrequencyConverter::fromBeats, this, _1, _2));
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&FrequencyConverter::fromControl, this, _1, _2));
    converters.emplace(MaximCommon::FormType::NOTE, std::bind(&FrequencyConverter::fromNote, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SAMPLES, std::bind(&FrequencyConverter::fromSamples, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SECONDS, std::bind(&FrequencyConverter::fromSeconds, this, _1, _2));
}

std::unique_ptr<FrequencyConverter> FrequencyConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<FrequencyConverter>(ctx, module);
}

llvm::Value* FrequencyConverter::fromBeats(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateLoad(ctx()->beatsPerSecond()),
        b.CreateFMul(val, ctx()->constFloatVec(60))
    );
}

llvm::Value* FrequencyConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::pow, val->getType());

    auto &b = method->builder();
    return b.CreateCall(powIntrinsic, {
        ctx()->constFloatVec(20000),
        val
    });
}

llvm::Value* FrequencyConverter::fromNote(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::pow, val->getType());

    auto &b = method->builder();
    return b.CreateFMul(
        ctx()->constFloatVec(440),
        b.CreateCall(powIntrinsic, {
            ctx()->constFloatVec(2),
            b.CreateFDiv(
                b.CreateFSub(val, ctx()->constFloatVec(69)),
                ctx()->constFloatVec(12)
            )
        })
    );
}

llvm::Value* FrequencyConverter::fromSamples(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(ctx()->constFloatVec(ctx()->sampleRate), val);
}

llvm::Value* FrequencyConverter::fromSeconds(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(ctx()->constFloatVec(1), val);
}
