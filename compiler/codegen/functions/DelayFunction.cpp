#include "DelayFunction.h"

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

DelayFunction::DelayFunction(MaximContext *context)
    : Function(context, "delay", context->numType(),
               {Parameter(context->numType(), false, false),
                Parameter(context->numType(), false, false),
                Parameter(context->numType(), true, true)},
               nullptr, getContextType(context)){

}

std::unique_ptr<DelayFunction> DelayFunction::create(MaximContext *context) {
    return std::make_unique<DelayFunction>(context);
}

std::unique_ptr<Value> DelayFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                               llvm::Function *func, llvm::Module *module) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::minnum, {context()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::maxnum, {context()->numType()->vecType()});

    auto inputNum = dynamic_cast<Num*>(params[0].get());
    auto delayNum = dynamic_cast<Num*>(params[1].get());
    auto reserveNum = dynamic_cast<Num*>(params[2].get());
    assert(inputNum && delayNum && reserveNum);

    auto inputVec = inputNum->vec(b);

    // calculate the number of samples of delay we have, used for modulo later
    auto delayPercentVec = delayNum->vec(b);
    auto reserveVec = reserveNum->vec(b);
    auto delaySecs = b.CreateBinOp(llvm::Instruction::BinaryOps::FMul, delayPercentVec, reserveVec, "delay.secs");
    auto minSecs = 1 / (float) context()->sampleRate;
    delaySecs = CreateCall(b, maxIntrinsic, {
        delaySecs, llvm::ConstantVector::getSplat(2, context()->constFloat(minSecs))
    }, "delay.secs");
    delaySecs = CreateCall(b, minIntrinsic, {
        delaySecs, llvm::ConstantVector::getSplat(2, context()->constFloat(1))
    }, "delay.secs");
    auto delaySamples = context()->secondsToSamples(delaySecs, b);

    // get pointers to index and sample array in struct
    auto indexPtr = b.CreateStructGEP(contextType(), funcContext, 0, "index.ptr");
    auto samplesPtr = b.CreateLoad(b.CreateStructGEP(contextType(), funcContext, 1, "samples.ptr.ptr"), "samples.ptr");

    auto currentIndex = b.CreateLoad(indexPtr, "index");
    auto placeSamplePtr = b.CreateGEP(samplesPtr, currentIndex, "write.ptr"); // returns a vec of pointers, extracted below

    // store current values at index
    auto storeStruct = b.CreateInsertValue(llvm::UndefValue::get(getChannelType(context())), inputNum->active(b), 0, "store.temp");

    auto leftStoreStruct = b.CreateInsertValue(storeStruct, b.CreateExtractElement(inputVec, (uint64_t) 0, "input.left"), 1, "store.left");
    auto leftStorePtr = b.CreateExtractElement(placeSamplePtr, (uint64_t) 0, "write.ptr.left");
    b.CreateStore(leftStoreStruct, leftStorePtr);

    auto rightStoreStruct = b.CreateInsertValue(storeStruct, b.CreateExtractElement(inputVec, (uint64_t) 1, "input.right"), 1, "store.right");
    auto rightStorePtr = b.CreateExtractElement(placeSamplePtr, (uint64_t) 1, "write.ptr.right");
    b.CreateStore(rightStoreStruct, rightStorePtr);

    // increment index
    auto nextIndex = b.CreateBinOp(llvm::Instruction::BinaryOps::Add, currentIndex, llvm::ConstantVector::getSplat(2, context()->constInt(64, 1, false)), "index.incr");
    nextIndex = b.CreateBinOp(llvm::Instruction::BinaryOps::URem, nextIndex, delaySamples, "index.mod");
    b.CreateStore(nextIndex, indexPtr);

    auto readSamplePtr = b.CreateGEP(samplesPtr, nextIndex, "read.ptr"); // returns a vec of pointers

    // read values at next index
    auto leftReadPtr = b.CreateExtractElement(readSamplePtr, (uint64_t) 0, "read.ptr.left");
    auto leftVal = b.CreateLoad(leftReadPtr, "read.left");
    auto rightReadPtr = b.CreateExtractElement(readSamplePtr, (uint64_t) 1, "read.ptr.right");
    auto rightVal = b.CreateLoad(rightReadPtr, "read.right");

    auto leftActive = b.CreateExtractValue(leftVal, {0}, "read.left.active");
    auto leftVec = b.CreateExtractValue(leftVal, {1}, "read.left.vec");
    auto rightActive = b.CreateExtractValue(rightVal, {0}, "read.right.active");
    auto rightVec = b.CreateExtractValue(rightVal, {1}, "read.right.vec");

    auto resultActive = b.CreateOr(leftActive, rightActive, "result.active");
    auto resultVec = b.CreateInsertElement(llvm::UndefValue::get(context()->numType()->vecType()), leftVec, (uint64_t) 0, "result.vec.temp");
    resultVec = b.CreateInsertElement(resultVec, rightVec, 1, "result.vec");

    auto undefPos = SourcePos(-1, -1);
    return inputNum->withVec(b, resultVec, undefPos, undefPos)->withActive(b, resultActive, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> DelayFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.insert(providedArgs.begin() + 1, Num::create(context(), 1, 1, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

std::unique_ptr<Instantiable> DelayFunction::generateCall(std::vector<std::unique_ptr<Value>> args) {
    auto reserveNum = dynamic_cast<Num*>(args[2].get());
    assert(reserveNum);

    auto constNum = llvm::cast<llvm::Constant>(reserveNum->get());
    auto constVec = constNum->getAggregateElement((unsigned) 0);

    auto leftSecs = llvm::cast<llvm::ConstantFP>(constVec->getAggregateElement((unsigned) 0))->getValueAPF().convertToFloat();
    auto rightSecs = llvm::cast<llvm::ConstantFP>(constVec->getAggregateElement((unsigned) 1))->getValueAPF().convertToFloat();

    if (leftSecs < 0) leftSecs = 0;
    if (rightSecs < 0) rightSecs = 0;

    auto leftSamples = context()->secondsToSamples(leftSecs);
    auto rightSamples = context()->secondsToSamples(rightSecs);
    return std::make_unique<FunctionCall>(leftSamples < 1 ? 1 : leftSamples, rightSamples < 1 ? 1 : rightSamples);
}

llvm::StructType* DelayFunction::getChannelType(MaximContext *ctx) {
    return llvm::StructType::get(ctx->llvm(), {
        llvm::Type::getInt1Ty(ctx->llvm()),
        llvm::Type::getFloatTy(ctx->llvm())
    }, false);
}

llvm::StructType* DelayFunction::getContextType(MaximContext *ctx) {
    auto channelPtrType = llvm::PointerType::get(getChannelType(ctx), 0);
    return llvm::StructType::get(ctx->llvm(), {
        llvm::VectorType::get(llvm::Type::getInt64Ty(ctx->llvm()), 2), // current index in sample arrays
        llvm::VectorType::get(channelPtrType, 2)                       // pointer to samples
    }, false);
}

DelayFunction::FunctionCall::FunctionCall(uint64_t leftDelaySize, uint64_t rightDelaySize)
    : leftDelaySize(leftDelaySize), rightDelaySize(rightDelaySize) {
}

llvm::Constant* DelayFunction::FunctionCall::getInitialVal(MaximContext *ctx) {
    auto ctxType = getContextType(ctx);

    return llvm::ConstantStruct::get(ctxType, {
        llvm::ConstantVector::getSplat(2, ctx->constInt(64, 0, false)),
        llvm::UndefValue::get(ctxType->getStructElementType(1))
    });
}

void DelayFunction::FunctionCall::initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) {
    auto channelType = getChannelType(ctx);
    auto channelPtrType = llvm::PointerType::get(channelType, 0);

    auto leftArray = llvm::ArrayType::get(channelType, leftDelaySize);
    auto leftGlobalArray = new llvm::GlobalVariable(
        *module, leftArray, false, llvm::GlobalValue::LinkageTypes::InternalLinkage,
        llvm::UndefValue::get(leftArray), "delay.left"
    );
    auto leftFirstPtr = b.CreateGEP(leftGlobalArray, {ctx->constInt(64, 0, false), ctx->constInt(32, 0, false)}, "delay.left.ptr");

    auto rightArray = llvm::ArrayType::get(channelType, rightDelaySize);
    auto rightGlobalArray = new llvm::GlobalVariable(
        *module, rightArray, false, llvm::GlobalValue::LinkageTypes::InternalLinkage,
        llvm::UndefValue::get(rightArray), "delay.right"
    );
    auto rightFirstPtr = b.CreateGEP(rightGlobalArray, {ctx->constInt(64, 0, false), ctx->constInt(32, 0, false)}, "delay.right.ptr");

    auto ptrVec = b.CreateInsertElement(llvm::UndefValue::get(llvm::VectorType::get(channelPtrType, 2)), leftFirstPtr, (uint64_t) 0, "delay.ptr.temp");
    ptrVec = b.CreateInsertElement(ptrVec, rightFirstPtr, 1, "delay.ptr");

    auto vecPtr = b.CreateStructGEP(getContextType(ctx), ptr, 1, "delay.ctx.ptr");
    b.CreateStore(ptrVec, vecPtr);
}

llvm::Type* DelayFunction::FunctionCall::type(MaximContext *ctx) const {
    return getContextType(ctx);
}
