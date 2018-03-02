#include "Converter.h"

#include "MaximContext.h"
#include "Num.h"
#include "ComposableModuleClassMethod.h"

using namespace MaximCodegen;

Converter::Converter(MaximContext *ctx, llvm::Module *module, MaximCommon::FormType toType)
    : ComposableModuleClass(ctx, module, "converter." + MaximCommon::formType2String(toType)), _toType(toType) {

    _callMethod = std::make_unique<ComposableModuleClassMethod>(this, "call",
                                                                ctx->numType()->get(),
                                                                std::vector<llvm::Type*>{ctx->numType()->get()});
}

void Converter::generate() {
    auto undefPos = SourcePos(-1, -1);
    auto numInput = Num::create(ctx(), _callMethod->arg(0), undefPos, undefPos);

    auto &b = _callMethod->builder();

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
        auto newNum = numInput->withVec(b, newVec, undefPos, undefPos)->withForm(b, _toType, undefPos, undefPos);
        b.CreateRet(newNum->get());
    }

    b.SetInsertPoint(defaultBlock);
    auto defaultNum = numInput->withForm(b, _toType, undefPos, undefPos);
    b.CreateRet(defaultNum->get());

    complete();
}

std::unique_ptr<Num> Converter::call(ComposableModuleClassMethod *method, std::unique_ptr<Num> value,
                                     SourcePos startPos, SourcePos endPos) {
    // hot path if there aren't any converters, just return the input with the target form
    if (converters.empty()) {
        return value->withForm(method->builder(), _toType, startPos, endPos);
    }

    // if the input is constant, constant-fold the converter
    auto realVal = value->get();
    if (auto constVal = llvm::dyn_cast<llvm::Constant>(realVal)) {
        auto formVal = llvm::cast<llvm::ConstantInt>(constVal->getAggregateElement(1))->getZExtValue();
        auto formConverter = converters.find((MaximCommon::FormType) formVal);

        if (formConverter == converters.end()) {
            return value->withForm(method->builder(), _toType, startPos, endPos);
        } else {
            auto vecVal = constVal->getAggregateElement((unsigned int) 0);
            auto converterFunc = formConverter->second;
            auto newVec = (this->*converterFunc)(method, vecVal);
            return value->withVec(method->builder(), newVec, startPos, endPos)->withForm(method->builder(), _toType,
                                                                                         startPos, endPos);
        }
    }

    // otherwise call the generated function
    auto entryIndex = method->moduleClass()->addEntry(this);
    auto result = method->callInto(entryIndex, {realVal}, _callMethod.get(), "convertresult");
    return Num::create(ctx(), result, startPos, endPos);
}
