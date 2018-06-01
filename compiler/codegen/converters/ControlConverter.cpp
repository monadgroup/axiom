#include "ControlConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

ControlConverter::ControlConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::CONTROL) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::BEATS, std::bind(&ControlConverter::fromBeats, this, _1, _2));
    converters.emplace(MaximCommon::FormType::DB, std::bind(&ControlConverter::fromDb, this, _1, _2));
    converters.emplace(MaximCommon::FormType::FREQUENCY, std::bind(&ControlConverter::fromFrequency, this, _1, _2));
    converters.emplace(MaximCommon::FormType::NOTE, std::bind(&ControlConverter::fromNote, this, _1, _2));
    converters.emplace(MaximCommon::FormType::OSCILLATOR, std::bind(&ControlConverter::fromOscillator, this, _1, _2));
    converters.emplace(MaximCommon::FormType::Q, std::bind(&ControlConverter::fromQ, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SAMPLES, std::bind(&ControlConverter::fromSamples, this, _1, _2));
    converters.emplace(MaximCommon::FormType::SECONDS, std::bind(&ControlConverter::fromSeconds, this, _1, _2));
}

std::unique_ptr<ControlConverter> ControlConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ControlConverter>(ctx, module);
}

llvm::Value* ControlConverter::fromBeats(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFMul(val, ctx()->constFloatVec(1.1)),
        b.CreateFAdd(val, ctx()->constFloatVec(0.8))
    );
}

llvm::Value* ControlConverter::fromDb(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto powIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow, val->getType());

    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateCall(powIntrinsic, {
            ctx()->constFloatVec(10),
            b.CreateFDiv(val, ctx()->constFloatVec(20))
        }),
        ctx()->constFloatVec(2)
    );
}

llvm::Value* ControlConverter::fromFrequency(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto logIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::log, val->getType());

    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateCall(logIntrinsic, {val}),
        b.CreateCall(logIntrinsic, {ctx()->constFloatVec(20000)})
    );
}

llvm::Value* ControlConverter::fromNote(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(val, ctx()->constFloatVec(127));
}

llvm::Value* ControlConverter::fromOscillator(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFAdd(
        b.CreateFMul(val, ctx()->constFloatVec(0.5)),
        ctx()->constFloatVec(0.5)
    );
}

llvm::Value* ControlConverter::fromQ(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFSub(val, ctx()->constFloatVec(0.5)),
        b.CreateFMul(val, ctx()->constFloatVec(0.999f))
    );
}

llvm::Value* ControlConverter::fromSamples(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFMul(val, ctx()->constFloatVec(1.1)),
        b.CreateFAdd(val, ctx()->constFloatVec(0.5f * ctx()->sampleRate))
    );
}

llvm::Value* ControlConverter::fromSeconds(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFDiv(
        b.CreateFMul(val, ctx()->constFloatVec(1.1)),
        b.CreateFAdd(val, ctx()->constFloatVec(0.5))
    );
}
