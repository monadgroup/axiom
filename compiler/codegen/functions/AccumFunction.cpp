#include "AccumFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

AccumFunction::AccumFunction(MaximContext *context)
    : Function(context, "accum", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, true)},
               nullptr, context->numType()->vecType()) {
}

std::unique_ptr<AccumFunction> AccumFunction::create(MaximContext *context) {
    return std::make_unique<AccumFunction>(context);
}

std::unique_ptr<Value> AccumFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                               llvm::Function *func, llvm::Module *module) {
    auto xVal = dynamic_cast<Num *>(params[0].get());
    auto gateVal = dynamic_cast<Num *>(params[1].get());
    auto baseVal = dynamic_cast<Num *>(params[2].get());

    auto accumVec = b.CreateLoad(funcContext, "accum");
    auto incrementedVec = b.CreateFAdd(accumVec, xVal->vec(b), "accum.incr");

    auto gateBool = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        gateVal->vec(b),
        llvm::ConstantVector::getSplat(2, context()->constFloat(0)),
        "gatebool"
    );
    auto floatGate = b.CreateUIToFP(
        gateBool,
        context()->numType()->vecType(),
        "gatefloat"
    );
    auto invGate = b.CreateFSub(
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        floatGate,
        "gatefloat.invert"
    );

    auto accumContrib = b.CreateFMul(incrementedVec, floatGate, "accum.mix");
    auto baseContrib = b.CreateFMul(baseVal->vec(b), invGate, "base.mix");
    auto newAccum = b.CreateFAdd(accumContrib, baseContrib, "newaccum");
    b.CreateStore(newAccum, funcContext);

    auto activeFlag = b.CreateOr(
        b.CreateExtractElement(gateBool, (uint64_t) 0, "gate.left"),
        b.CreateExtractElement(gateBool, (uint64_t) 1, "gate.right"),
        "active"
    );
    activeFlag = b.CreateAnd(gateVal->active(b), activeFlag, "active");

    auto undefPos = SourcePos(-1, -1);
    return xVal->withVec(b, newAccum, undefPos, undefPos)->withActive(b, activeFlag, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> AccumFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(Num::create(context(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

std::unique_ptr<Instantiable> AccumFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<FunctionCall>();
}

llvm::Constant *AccumFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantVector::getSplat(2, ctx->constFloat(0));
}

llvm::Type *AccumFunction::FunctionCall::type(MaximContext *ctx) const {
    return ctx->numType()->vecType();
}
