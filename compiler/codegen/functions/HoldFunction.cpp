#include "HoldFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

HoldFunction::HoldFunction(MaximContext *context)
    : Function(context, "hold", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, true)},
               nullptr, getContextType(context)) {

}

std::unique_ptr<HoldFunction> HoldFunction::create(MaximContext *context) {
    return std::make_unique<HoldFunction>(context);
}

std::unique_ptr<Value> HoldFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                              std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                              llvm::Function *func, llvm::Module *module) {
    auto contextType = getContextType(context());
    auto xVal = dynamic_cast<Num*>(params[0].get());
    auto gateVal = dynamic_cast<Num*>(params[1].get());
    auto elseVal = dynamic_cast<Num*>(params[2].get());
    assert(xVal && gateVal && elseVal);

    auto valPtr = b.CreateStructGEP(contextType, funcContext, 0, "val.ptr");
    auto gatePtr = b.CreateStructGEP(contextType, funcContext, 1, "gate.ptr");

    auto gateBool = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        gateVal->vec(b), llvm::ConstantVector::getSplat(2, context()->constFloat(0))
    );

    auto lastGate = b.CreateLoad(gatePtr, "lastgate");
    b.CreateStore(gateBool, gatePtr);
    auto isRising = b.CreateUIToFP(
        b.CreateICmp(llvm::CmpInst::Predicate::ICMP_UGT, gateBool, lastGate, "isrising"),
        context()->numType()->vecType(),
        "isrising.float"
    );
    auto invRising = b.CreateFSub(
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
        isRising,
        "isrising.invert"
    );

    auto currentVec = xVal->vec(b);
    auto currentContrib = b.CreateFMul(currentVec, isRising, "currentval.mix");
    auto lastVal = b.CreateLoad(valPtr, "lastval");
    auto lastContrib = b.CreateFMul(lastVal, invRising, "lastval.mix");

    auto newVal = b.CreateFAdd(currentContrib, lastContrib, "newval");
    b.CreateStore(newVal, valPtr);

    auto gateFloat = b.CreateUIToFP(gateBool, context()->numType()->vecType(), "gate.float");
    auto invGate = b.CreateFSub(
        llvm::ConstantVector::getSplat(2, context()->constFloat(1)),
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

    auto undefPos = SourcePos(-1, -1);
    return xVal->withVec(b, resultVec, undefPos, undefPos)->withActive(b, activeFlag, undefPos, undefPos);

}

std::vector<std::unique_ptr<Value>> HoldFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.push_back(Num::create(context(), 0, 0, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

std::unique_ptr<Instantiable> HoldFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    return std::make_unique<FunctionCall>();
}

llvm::StructType* HoldFunction::getContextType(MaximContext *ctx) {
    return llvm::StructType::get(ctx->llvm(), {
        ctx->numType()->vecType(),                                   // stored value
        llvm::VectorType::get(llvm::Type::getInt1Ty(ctx->llvm()), 2) // last gate value
    });
}

llvm::Constant* HoldFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    return llvm::ConstantStruct::get(getContextType(ctx), {
        llvm::UndefValue::get(ctx->numType()->vecType()),
        llvm::ConstantVector::getSplat(2, ctx->constInt(1, 0, false))
    });
}

llvm::Type* HoldFunction::FunctionCall::type(MaximContext *ctx) const {
    return getContextType(ctx);
}
