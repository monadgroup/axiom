#include "Converter.h"

#include "MaximContext.h"
#include "Num.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

Converter::Converter(MaximContext *ctx, llvm::Module *module, MaximCommon::FormType toType)
    : ComposableModuleClass(ctx, module, "converter." + MaximCommon::formType2String(toType)), _toType(toType) {

    _callMethod = std::make_unique<ComposableModuleClassMethod>(this, "call",
                                                                ctx->numType()->get(),
                                                                std::vector<llvm::Type *>{ctx->numType()->get()});
}

void Converter::generate() {
    auto undefPos = SourcePos(-1, -1);
    auto numInputPtr = _callMethod->allocaBuilder().CreateAlloca(ctx()->numType()->get(), nullptr, "input");
    auto &b = _callMethod->builder();

    b.CreateStore(_callMethod->arg(0), numInputPtr);
    auto numInput = Num::create(ctx(), numInputPtr, undefPos, undefPos);

    auto func = _callMethod->get(_callMethod->moduleClass()->module());
    auto numForm = numInput->form(b);
    auto numVec = numInput->vec(b);

    auto defaultBlock = llvm::BasicBlock::Create(ctx()->llvm(), "default", func);

    auto formSwitch = b.CreateSwitch(numForm, defaultBlock, (unsigned int) converters.size());
    for (const auto &pair : converters) {
        auto blockName = "branch." + MaximCommon::formType2String(pair.first);
        auto converterBlock = llvm::BasicBlock::Create(ctx()->llvm(), blockName, func);
        formSwitch->addCase(
            llvm::ConstantInt::get(ctx()->numType()->formType(), (uint64_t) pair.first, false),
            converterBlock
        );
        b.SetInsertPoint(converterBlock);

        auto convertFunc = pair.second;
        auto newVec = (this->*convertFunc)(_callMethod.get(), numVec);
        numInput->setVec(b, newVec);
        numInput->setForm(b, _toType);
        b.CreateRet(b.CreateLoad(numInput->get(), "conv.deref"));
    }

    b.SetInsertPoint(defaultBlock);
    numInput->setForm(b, _toType);
    b.CreateRet(b.CreateLoad(numInput->get(), "conv.deref"));

    complete();
}

std::unique_ptr<Num> Converter::call(ComposableModuleClassMethod *method, std::unique_ptr<Num> value,
                                     SourcePos startPos, SourcePos endPos) {
    // hot path if there aren't any converters, just return the input with the target form
    if (converters.empty()) {
        value->setForm(method->builder(), _toType);
        return std::move(value);
    }

    // call the generated function
    auto regVal = method->builder().CreateLoad(value->get(), "conv.deref");
    auto entryIndex = method->moduleClass()->addEntry(this);
    auto result = method->callInto(entryIndex, {regVal}, _callMethod.get(), "convertresult");
    return Num::create(ctx(), result, startPos, endPos);
}
