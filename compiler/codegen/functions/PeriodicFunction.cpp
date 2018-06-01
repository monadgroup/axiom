#include "PeriodicFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

PeriodicFunction::PeriodicFunction(MaximContext *ctx, llvm::Module *module, std::string name)
    : Function(ctx, module, name, ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, true)},
               nullptr) {

}

std::unique_ptr<Value>
PeriodicFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                           std::unique_ptr<VarArg> vararg) {
    auto freqVal = dynamic_cast<Num *>(params[0].get());
    auto phaseOffsetVal = dynamic_cast<Num *>(params[1].get());
    assert(freqVal && phaseOffsetVal);

    auto &b = method->builder();
    auto funcContext = method->getEntryPointer(addEntry(ctx()->numType()->vecType()), "ctx");

    auto phaseVec = b.CreateLoad(funcContext, "phase");

    // offset phase and store new value
    auto phaseOffset = b.CreateFDiv(freqVal->vec(b),
                                    ctx()->constFloatVec(ctx()->sampleRate),
                                    "phaseoffset");
    auto newPhase = b.CreateFAdd(phaseVec, phaseOffset, "newphase");
    auto modPhase = b.CreateFRem(
        newPhase,
        ctx()->constFloatVec(2),
        "modphase"
    );
    b.CreateStore(modPhase, funcContext);

    // calculate result
    auto inputPhase = b.CreateFAdd(phaseOffsetVal->vec(b), phaseVec, "inputphase");
    auto resultVec = nextValue(method, inputPhase);

    // create result number
    auto isActive = b.CreateOr(freqVal->active(b), phaseOffsetVal->active(b), "active");
    auto newNum = Num::create(ctx(), method->allocaBuilder());
    newNum->setVec(b, resultVec);
    newNum->setForm(b, MaximCommon::FormType::OSCILLATOR);
    newNum->setActive(b, isActive);
    return std::move(newNum);
}

std::vector<std::unique_ptr<Value>>
PeriodicFunction::mapArguments(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 2) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(
            Num::create(ctx(), method->allocaBuilder(), 0, 0, MaximCommon::FormType::NONE, true, undefPos, undefPos));
    }
    return providedArgs;
}
