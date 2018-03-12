#include "HoldFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

HoldFunction::HoldFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "hold", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, true)},
               nullptr) {

}

std::unique_ptr<HoldFunction> HoldFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<HoldFunction>(ctx, module);
}

std::unique_ptr<Value>
HoldFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                       std::unique_ptr<VarArg> vararg) {
    auto xVal = dynamic_cast<Num *>(params[0].get());
    auto gateVal = dynamic_cast<Num *>(params[1].get());
    auto elseVal = dynamic_cast<Num *>(params[2].get());
    assert(xVal && gateVal && elseVal);

    auto &b = method->builder();
    auto contextType = llvm::StructType::get(ctx()->llvm(), {
        ctx()->numType()->vecType(),                                   // stored value
        llvm::VectorType::get(llvm::Type::getInt1Ty(ctx()->llvm()), 2) // last gate value
    });
    auto funcContext = method->getEntryPointer(addEntry(contextType), "ctx");

    auto valPtr = b.CreateStructGEP(contextType, funcContext, 0, "val.ptr");
    auto gatePtr = b.CreateStructGEP(contextType, funcContext, 1, "gate.ptr");

    auto gateBool = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        gateVal->vec(b), llvm::ConstantVector::getSplat(2, ctx()->constFloat(0)),
        "gatebool"
    );

    auto lastGate = b.CreateLoad(gatePtr, "lastgate");
    b.CreateStore(gateBool, gatePtr);
    auto isRising = b.CreateUIToFP(
        b.CreateICmp(llvm::CmpInst::Predicate::ICMP_UGT, gateBool, lastGate, "isrising"),
        ctx()->numType()->vecType(),
        "isrising.float"
    );
    auto invRising = b.CreateFSub(
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        isRising,
        "isrising.invert"
    );

    auto currentVec = xVal->vec(b);
    auto currentContrib = b.CreateFMul(currentVec, isRising, "currentval.mix");
    auto lastVal = b.CreateLoad(valPtr, "lastval");
    auto lastContrib = b.CreateFMul(lastVal, invRising, "lastval.mix");

    auto newVal = b.CreateFAdd(currentContrib, lastContrib, "newval");
    b.CreateStore(newVal, valPtr);

    auto gateFloat = b.CreateUIToFP(gateBool, ctx()->numType()->vecType(), "gate.float");
    auto invGate = b.CreateFSub(
        llvm::ConstantVector::getSplat(2, ctx()->constFloat(1)),
        gateFloat,
        "gate.invert"
    );

    auto holdedContrib = b.CreateFMul(newVal, gateFloat, "holded.mix");
    auto elseVec = elseVal->vec(b);
    auto elseContrib = b.CreateFMul(elseVec, invGate, "else.mix");
    auto resultVec = b.CreateFAdd(holdedContrib, elseContrib, "resultvec");

    auto activeFlag = b.CreateOr(
        b.CreateExtractElement(gateBool, (uint64_t) 0, "gate.left"),
        b.CreateExtractElement(gateBool, (uint64_t) 1, "gate.right"),
        "active"
    );
    activeFlag = b.CreateAnd(gateVal->active(b), activeFlag, "active");

    xVal->setVec(b, resultVec);
    xVal->setActive(b, activeFlag);
    return xVal->clone();
}

std::vector<std::unique_ptr<Value>> HoldFunction::mapArguments(ComposableModuleClassMethod *method, std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(Num::create(ctx(), method->allocaBuilder(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}
