#include <llvm/Support/raw_ostream.h>
#include "DelayFunction.h"

#include "../MaximContext.h"
#include "../Num.h"
#include "../ComposableModuleClassMethod.h"

using namespace MaximCodegen;

DelayFunction::DelayFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "delay", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), true, true)}, nullptr, false) {

}

std::unique_ptr<DelayFunction> DelayFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<DelayFunction>(ctx, module);
}

std::unique_ptr<Value> DelayFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) {
    auto minIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum,
                                                        {ctx()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::maxnum,
                                                        {ctx()->numType()->vecType()});

    auto channelType = getChannelType();
    auto ctxType = getContextType();

    auto entryIndex = method->moduleClass()->addEntry(llvm::ConstantStruct::get(ctxType, {
        llvm::ConstantVector::getSplat(2, ctx()->constInt(64, 0, false)),
        llvm::UndefValue::get(ctxType->getStructElementType(1))
    }));
    auto entryPtr = method->getEntryPointer(entryIndex, "delaydat");

    auto inputNum = dynamic_cast<Num *>(params[0].get());
    auto delayNum = dynamic_cast<Num *>(params[1].get());
    auto reserveNum = dynamic_cast<Num *>(params[2].get());
    assert(inputNum && delayNum && reserveNum);

    auto &b = method->builder();

    auto inputVec = inputNum->vec(b);

    // calculate the number of samples of delay we have, used for modulo later
    auto delayPercentVec = delayNum->vec(b);
    auto reserveVec = reserveNum->vec(b);
    auto delaySecs = b.CreateBinOp(llvm::Instruction::BinaryOps::FMul, delayPercentVec, reserveVec, "delay.secs");
    auto minSecs = 1 / (float) ctx()->sampleRate;
    delaySecs = CreateCall(b, maxIntrinsic, {
        delaySecs, llvm::ConstantVector::getSplat(2, ctx()->constFloat(minSecs))
    }, "delay.secs");
    delaySecs = CreateCall(b, minIntrinsic, {
        delaySecs, llvm::ConstantVector::getSplat(2, ctx()->constFloat(1))
    }, "delay.secs");
    auto delaySamples = ctx()->secondsToSamples(delaySecs, b);

    // get pointers to index and sample array in struct
    auto indexPtr = b.CreateStructGEP(ctxType, entryPtr, 0, "index.ptr");
    auto samplesPtr = b.CreateLoad(b.CreateStructGEP(ctxType, entryPtr, 1, "samples.ptr.ptr"), "samples.ptr");

    auto currentIndex = b.CreateLoad(indexPtr, "index");
    auto placeSamplePtr = b.CreateGEP(samplesPtr, currentIndex,
                                      "write.ptr"); // returns a vec of pointers, extracted below

    // store current values at index
    auto storeStruct = b.CreateInsertValue(llvm::UndefValue::get(channelType), inputNum->active(b), 0,
                                           "store.temp");

    auto leftStoreStruct = b.CreateInsertValue(storeStruct,
                                               b.CreateExtractElement(inputVec, (uint64_t) 0, "input.left"), 1,
                                               "store.left");
    auto leftStorePtr = b.CreateExtractElement(placeSamplePtr, (uint64_t) 0, "write.ptr.left");
    b.CreateStore(leftStoreStruct, leftStorePtr);

    auto rightStoreStruct = b.CreateInsertValue(storeStruct,
                                                b.CreateExtractElement(inputVec, (uint64_t) 1, "input.right"), 1,
                                                "store.right");
    auto rightStorePtr = b.CreateExtractElement(placeSamplePtr, (uint64_t) 1, "write.ptr.right");
    b.CreateStore(rightStoreStruct, rightStorePtr);

    // increment index
    auto nextIndex = b.CreateBinOp(llvm::Instruction::BinaryOps::Add, currentIndex,
                                   llvm::ConstantVector::getSplat(2, ctx()->constInt(64, 1, false)), "index.incr");
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
    auto resultVec = b.CreateInsertElement(llvm::UndefValue::get(ctx()->numType()->vecType()), leftVec,
                                           (uint64_t) 0, "result.vec.temp");
    resultVec = b.CreateInsertElement(resultVec, rightVec, 1, "result.vec");

    auto undefPos = SourcePos(-1, -1);
    return inputNum->withVec(b, resultVec, undefPos, undefPos)->withActive(b, resultActive, undefPos, undefPos);
}

std::vector<std::unique_ptr<Value>> DelayFunction::mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        auto undefPos = SourcePos(-1, -1);
        providedArgs.insert(providedArgs.begin() + 1,
                            Num::create(ctx(), 1, 1, MaximCommon::FormType::LINEAR, true, undefPos, undefPos));
    }
    return providedArgs;
}

void DelayFunction::sampleArguments(ComposableModuleClassMethod *method, size_t index,
                                    const std::vector<std::unique_ptr<Value>> &args,
                                    const std::vector<std::unique_ptr<Value>> &varargs) {
    llvm::ConstantFolder folder;
    auto delayNum = dynamic_cast<Num *>(args[1].get());
    auto reserveNum = dynamic_cast<Num *>(args[2].get());
    assert(delayNum && reserveNum);

    auto constReserve = llvm::cast<llvm::Constant>(reserveNum->get());
    auto reserveVec = constReserve->getAggregateElement((unsigned) 0);

    // if delay is constant, premultiply it
    auto constDelay = llvm::dyn_cast<llvm::Constant>(delayNum->get());
    if (constDelay) {
        auto delayVec = constDelay->getAggregateElement((unsigned) 0);
        reserveVec = folder.CreateFMul(reserveVec, delayVec);
    }

    auto leftSecs = llvm::cast<llvm::ConstantFP>(
        reserveVec->getAggregateElement((unsigned) 0))->getValueAPF().convertToFloat();
    auto rightSecs = llvm::cast<llvm::ConstantFP>(
        reserveVec->getAggregateElement((unsigned) 1))->getValueAPF().convertToFloat();

    if (leftSecs < 0) leftSecs = 0;
    if (rightSecs < 0) rightSecs = 0;

    auto leftSamples = ctx()->secondsToSamples(leftSecs);
    auto rightSamples = ctx()->secondsToSamples(rightSecs);

    auto channelType = getChannelType();
    auto channelPtrType = llvm::PointerType::get(channelType, 0);
    auto &b = method->moduleClass()->constructor()->builder();

    // create global variables that store the values
    auto leftArrayType = llvm::ArrayType::get(channelType, leftSamples);
    auto leftGlobalVariable = new llvm::GlobalVariable(
        *method->moduleClass()->module(), leftArrayType, false, llvm::GlobalValue::LinkageTypes::InternalLinkage,
        llvm::UndefValue::get(leftArrayType), "delay.left"
    );

    auto rightArrayType = llvm::ArrayType::get(channelType, rightSamples);
    auto rightGlobalVariable = new llvm::GlobalVariable(
        *method->moduleClass()->module(), rightArrayType, false, llvm::GlobalValue::LinkageTypes::InternalLinkage,
        llvm::UndefValue::get(rightArrayType), "delay.right"
    );

    // get a ptr vector to the first item in each array, to put into the context
    auto leftFirstPtr = b.CreateGEP(leftGlobalVariable, {ctx()->constInt(64, 0, false), ctx()->constInt(32, 0, false)}, "delayleft.ptr");
    auto rightFirstPtr = b.CreateGEP(rightGlobalVariable, {ctx()->constInt(64, 0, false), ctx()->constInt(32, 0, false)}, "delayright.ptr");

    auto ptrVec = b.CreateInsertElement(llvm::UndefValue::get(llvm::VectorType::get(channelPtrType, 2)), leftFirstPtr, (uint64_t) 0, "delay.ptr.tmp");
    ptrVec = b.CreateInsertElement(ptrVec, rightFirstPtr, 1, "delay.ptr");

    // store the ptr vec into the context object
    auto entryPtr = method->moduleClass()->cconstructor()->getEntryPointer(index, "delay");
    auto samplesPtr = b.CreateGEP(entryPtr, {
        ctx()->constInt(64, 0, false),
        ctx()->constInt(32, 0, false),
        ctx()->constInt(32, 1, false)
    }, "delay.ctx.ptr");
    b.CreateStore(ptrVec, samplesPtr);
}

llvm::StructType *DelayFunction::getChannelType() {
    return llvm::StructType::get(ctx()->llvm(), {
        llvm::Type::getInt1Ty(ctx()->llvm()),
        llvm::Type::getFloatTy(ctx()->llvm())
    }, false);
}

llvm::StructType *DelayFunction::getContextType() {
    auto channelPtrType = llvm::PointerType::get(getChannelType(), 0);
    return llvm::StructType::get(ctx()->llvm(), {
        llvm::VectorType::get(llvm::Type::getInt64Ty(ctx()->llvm()), 2), // current index in sample arrays
        llvm::VectorType::get(channelPtrType, 2)                       // pointer to samples
    }, false);
}
