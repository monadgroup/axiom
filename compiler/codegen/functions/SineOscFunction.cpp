#include "SineOscFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

SineOscFunction::SineOscFunction(MaximContext *context)
    : Function(context, "sineOsc", context->numType(),
               {Parameter(context->numType(), false, false), Parameter(context->numType(), false, true)},
                nullptr, context->numType()->vecType()) {

}

std::unique_ptr<SineOscFunction> SineOscFunction::create(MaximContext *context) {
    return std::make_unique<SineOscFunction>(context);
}

std::unique_ptr<Value> SineOscFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                 std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                 llvm::Function *func, llvm::Module *module) {
    auto sineFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::sin, {context()->numType()->vecType()});

    auto freqVal = dynamic_cast<Num*>(params[0].get());
    auto phaseOffsetVal = dynamic_cast<Num*>(params[1].get());
    auto phaseVec = b.CreateLoad(funcContext, "phase");

    auto inputPhase = b.CreateFAdd(phaseOffsetVal->vec(b), phaseVec, "sinephase");
    auto inputVal = b.CreateFMul(
        llvm::ConstantVector::getSplat(2, context()->constFloat(2 * M_PI)),
        inputPhase, "sineparam"
    );
    auto sineResult = CreateCall(b, sineFunc, {inputVal}, "sineval");

    auto phaseOffset = b.CreateFDiv(freqVal->vec(b), llvm::ConstantVector::getSplat(2, context()->constFloat(context()->sampleRate)), "phaseoffset");
    auto newPhase = b.CreateFAdd(phaseVec, phaseOffset, "newphase");
    b.CreateStore(newPhase, funcContext);

    auto isActive = b.CreateOr(freqVal->active(b), phaseOffsetVal->active(b), "active");
    auto undefPos = SourcePos(-1, -1);
    auto newNum = Num::create(context(), llvm::UndefValue::get(context()->numType()->get()), undefPos, undefPos);
    return newNum->withVec(b, sineResult, undefPos, undefPos)
                 ->withForm(b, MaximCommon::FormType::OSCILLATOR, undefPos, undefPos)
                 ->withActive(b, isActive, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> SineOscFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 2) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(Num::create(context(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

std::unique_ptr<Instantiable> SineOscFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<FunctionCall>();
}

llvm::Constant* SineOscFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantVector::get({ctx->constFloat(0), ctx->constFloat(0)});
}

llvm::Type* SineOscFunction::FunctionCall::type(MaximContext *ctx) const {
    return ctx->numType()->vecType();
}
