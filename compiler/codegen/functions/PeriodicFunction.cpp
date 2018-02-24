#include "PeriodicFunction.h"

#include <llvm/IR/Intrinsics.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

PeriodicFunction::PeriodicFunction(MaximContext *context, std::string name)
    : Function(context, std::move(name), context->numType(),
               {Parameter(context->numType(), false, false), Parameter(context->numType(), false, true)},
                nullptr, context->numType()->vecType()) {

}

std::unique_ptr<Value> PeriodicFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                  std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                  llvm::Function *func, llvm::Module *module) {
    auto freqVal = dynamic_cast<Num*>(params[0].get());
    auto phaseOffsetVal = dynamic_cast<Num*>(params[1].get());
    assert(freqVal && phaseOffsetVal);

    auto phaseVec = b.CreateLoad(funcContext, "phase");

    // offset phase and store new value
    auto phaseOffset = b.CreateFDiv(freqVal->vec(b), llvm::ConstantVector::getSplat(2, context()->constFloat(context()->sampleRate)), "phaseoffset");
    auto newPhase = b.CreateFAdd(phaseVec, phaseOffset, "newphase");
    auto modPhase = b.CreateFRem(
        newPhase,
        llvm::ConstantVector::getSplat(2, context()->constFloat(2)),
        "modphase"
    );
    b.CreateStore(modPhase, funcContext);

    // calculate result
    auto inputPhase = b.CreateFAdd(phaseOffsetVal->vec(b), phaseVec, "inputphase");
    auto resultVec = nextValue(inputPhase, b, module);

    // create result number
    auto isActive = b.CreateOr(freqVal->active(b), phaseOffsetVal->active(b), "active");
    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(context(), llvm::UndefValue::get(context()->numType()->get()), undefPos, undefPos);
    return newNum->withVec(b, resultVec, undefPos, undefPos)
                 ->withForm(b, MaximCommon::FormType::OSCILLATOR, undefPos, undefPos)
                 ->withActive(b, isActive, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> PeriodicFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 2) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(Num::create(context(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

std::unique_ptr<Instantiable> PeriodicFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<FunctionCall>();
}

llvm::Constant* PeriodicFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantVector::get({ctx->constFloat(0), ctx->constFloat(0)});
}

llvm::Type* PeriodicFunction::FunctionCall::type(MaximContext *ctx) const {
    return ctx->numType()->vecType();
}
