#include "NoteConverter.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

NoteConverter::NoteConverter(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Converter(ctx, module, MaximCommon::FormType::NOTE) {
    using namespace std::placeholders;
    converters.emplace(MaximCommon::FormType::CONTROL, std::bind(&NoteConverter::fromControl, this, _1, _2));
    converters.emplace(MaximCommon::FormType::FREQUENCY, std::bind(&NoteConverter::fromFrequency, this, _1, _2));
}

std::unique_ptr<NoteConverter> NoteConverter::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<NoteConverter>(ctx, module);
}

llvm::Value* NoteConverter::fromControl(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto &b = method->builder();
    return b.CreateFMul(val, ctx()->constFloatVec(127));
}

llvm::Value* NoteConverter::fromFrequency(MaximCodegen::ComposableModuleClassMethod *method, llvm::Value *val) {
    auto log2Intrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::log2, val->getType());

    auto &b = method->builder();
    return b.CreateFAdd(
        ctx()->constFloatVec(69),
        b.CreateFMul(
            ctx()->constFloatVec(12),
            b.CreateCall(log2Intrinsic, {
                b.CreateFDiv(val, ctx()->constFloatVec(440))
            })
        )
    );
}
