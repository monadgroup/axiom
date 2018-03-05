#include "AccumFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

AccumFunction::AccumFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "accum", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, true)},
               nullptr, false) {
}

std::unique_ptr<AccumFunction> AccumFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<AccumFunction>(ctx, module);
}

std::unique_ptr<Value>
AccumFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                        std::unique_ptr<VarArg> vararg) {
    auto xVal = dynamic_cast<Num *>(params[0].get());
    auto gateVal = dynamic_cast<Num *>(params[1].get());
    auto baseVal = dynamic_cast<Num *>(params[2].get());

    auto &b = method->builder();
    auto funcContext = method->getEntryPointer(addEntry(ctx()->constFloatVec(0)), "ctx");

    auto accumVec = b.CreateLoad(funcContext, "accum");
    auto incrementedVec = b.CreateFAdd(accumVec, xVal->vec(b), "accum.incr");

    auto gateBool = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        gateVal->vec(b),
        ctx()->constFloatVec(0),
        "gatebool"
    );
    auto floatGate = b.CreateUIToFP(
        gateBool,
        ctx()->numType()->vecType(),
        "gatefloat"
    );
    auto invGate = b.CreateFSub(
        ctx()->constFloatVec(1),
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
        providedArgs.push_back(Num::create(ctx(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}
