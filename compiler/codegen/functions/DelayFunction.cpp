#include "DelayFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

DelayFunction::DelayFunction(MaximCodegen::MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "delay", ctx->numType(),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, true)}, nullptr) {

}

std::unique_ptr<DelayFunction> DelayFunction::create(MaximCodegen::MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<DelayFunction>(ctx, module);
}

std::unique_ptr<Value> DelayFunction::generate(MaximCodegen::ComposableModuleClassMethod *method,
                                               const std::vector<std::unique_ptr<MaximCodegen::Value>> &params,
                                               std::unique_ptr<MaximCodegen::VarArg> vararg) {
    auto sizeType = llvm::Type::getInt64Ty(ctx()->llvm());
    auto channelType = llvm::StructType::get(ctx()->llvm(), {
        llvm::Type::getInt1Ty(ctx()->llvm()),
        llvm::Type::getFloatTy(ctx()->llvm())
    }, false);
    auto dataLayoutType = llvm::Type::getIntNTy(ctx()->llvm(), ctx()->dataLayout().getPointerSizeInBits(0));
    auto voidPtrType = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx()->llvm()), 0);

    auto minIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum,
                                                        {ctx()->numType()->vecType()});
    auto maxIntrinsic = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::maxnum,
                                                        {ctx()->numType()->vecType()});
    auto clzIntrinsic = llvm::Intrinsic::getDeclaration(module(), llvm::Intrinsic::ID::ctlz, {sizeType});
    auto reallocFunction = llvm::Function::Create(
        llvm::FunctionType::get(voidPtrType, {
            voidPtrType,
            dataLayoutType
        }, false),
        llvm::Function::ExternalLinkage, "realloc", module()
    );
    auto freeFunction = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()->llvm()), {voidPtrType}, false),
        llvm::Function::ExternalLinkage, "free", module()
    );

    // we need to store:
    //  - current lengths of buffers (in samples)
    //  - current index in sample arrays
    //  - pointer to samples

    // on evaluation:
    // if buffer length > 0:
    //   store new value at current position
    //   if delay length > 0: add 1 to current position, then modulo based on delay length
    //   read from current position and save as result
    // else:
    //  set result to be new value

    // get input length in samples
    // while input length in samples > buffer length
    //   double buffer length with realloc
    // todo: reduce buffer size if it makes sense?
    // note that applying the resize _after_ reading is important to make sure we don't get
    // some samples of nonsense, so we don't need to fill the buffer with zeros.

    // return result

    // create utility function for updating each channel
    auto channelUpdateType = llvm::FunctionType::get(channelType, {
        llvm::PointerType::get(sizeType, 0),                               // current position pointer
        llvm::PointerType::get(sizeType, 0),                               // current size pointer
        sizeType,                                                          // delay sample count
        sizeType,                                                          // reserve sample count
        llvm::PointerType::get(llvm::PointerType::get(channelType, 0), 0), // samples pointer pointer
        ctx()->numType()->activeType(),                                    // input active?
        llvm::Type::getFloatTy(ctx()->llvm()),                             // input value
    }, false);
    auto channelUpdateFunc = llvm::Function::Create(channelUpdateType, llvm::Function::InternalLinkage,
                                                    "maxim.util.channelUpdate", module());

    {
        auto currentPosPtr = channelUpdateFunc->arg_begin();
        auto currentSizePtr = channelUpdateFunc->arg_begin() + 1;
        auto delaySamples = channelUpdateFunc->arg_begin() + 2;
        auto reserveSamples = channelUpdateFunc->arg_begin() + 3;
        auto samplePtrPtr = channelUpdateFunc->arg_begin() + 4;
        auto inputActive = channelUpdateFunc->arg_begin() + 5;
        auto inputNum = channelUpdateFunc->arg_begin() + 6;

        auto entryBlock = llvm::BasicBlock::Create(ctx()->llvm(), "entry", channelUpdateFunc);
        auto existsTrueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "exists.true", channelUpdateFunc);
        auto delayTrueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "delay.true", channelUpdateFunc);
        auto delayContinueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "delay.continue", channelUpdateFunc);
        auto existsFalseBlock = llvm::BasicBlock::Create(ctx()->llvm(), "exists.false", channelUpdateFunc);
        auto resizeCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "resize.check", channelUpdateFunc);
        auto resizeExecBlock = llvm::BasicBlock::Create(ctx()->llvm(), "resize.exec", channelUpdateFunc);
        auto resizeContinueBlock = llvm::BasicBlock::Create(ctx()->llvm(), "resize.continue", channelUpdateFunc);

        llvm::IRBuilder<> b(entryBlock);

        auto resultPtr = b.CreateAlloca(channelType, nullptr, "result.ptr");
        auto samplePtr = b.CreateLoad(samplePtrPtr, "sampleptr");

        auto bufferSize = b.CreateLoad(currentSizePtr);
        auto bufferExists = b.CreateICmpUGT(bufferSize, ctx()->constInt(64, 0, false), "bufferexists");
        b.CreateCondBr(bufferExists, existsTrueBlock, existsFalseBlock);
        b.SetInsertPoint(existsTrueBlock);

        // store new value at current position
        auto currentPos = b.CreateLoad(currentPosPtr, "currentpos");
        auto activePtr = b.CreateGEP(samplePtr, {
            currentPos,
            ctx()->constInt(32, 0, false)
        }, "active.ptr");
        b.CreateStore(inputActive, activePtr);
        auto valPtr = b.CreateGEP(samplePtr, {
            currentPos,
            ctx()->constInt(32, 1, false)
        }, "val.ptr");
        b.CreateStore(inputNum, valPtr);

        auto hasDelay = b.CreateICmpUGT(delaySamples, ctx()->constInt(64, 0, false), "hasdelay");
        b.CreateCondBr(hasDelay, delayTrueBlock, delayContinueBlock);
        b.SetInsertPoint(delayTrueBlock);

        auto newPos = b.CreateAdd(currentPos, ctx()->constInt(64, 1, false), "newpos");
        newPos = b.CreateURem(newPos, delaySamples, "newpos");
        b.CreateStore(newPos, currentPosPtr);
        b.CreateBr(delayContinueBlock);
        b.SetInsertPoint(delayContinueBlock);

        currentPos = b.CreateLoad(currentPosPtr, "currentpos");
        auto resultVal = b.CreateLoad(b.CreateGEP(samplePtr, currentPos, "result.ptr"), "result");
        b.CreateStore(resultVal, resultPtr);
        b.CreateBr(resizeCheckBlock);
        b.SetInsertPoint(existsFalseBlock);

        auto resultActivePtr = b.CreateStructGEP(channelType, resultPtr, 0, "resultactive.ptr");
        b.CreateStore(inputActive, resultActivePtr);
        auto resultNumPtr = b.CreateStructGEP(channelType, resultPtr, 1, "resultnum.ptr");
        b.CreateStore(inputNum, resultNumPtr);
        b.CreateBr(resizeCheckBlock);
        b.SetInsertPoint(resizeCheckBlock);

        auto needsResize = b.CreateICmpUGT(reserveSamples, bufferSize, "needsresize");
        b.CreateCondBr(needsResize, resizeExecBlock, resizeContinueBlock);
        b.SetInsertPoint(resizeExecBlock);

        // calculate the next largest power of two of reserve size
        auto numSub = b.CreateSub(reserveSamples, ctx()->constInt(64, 1, false), "reservesub");
        auto clz = CreateCall(b, clzIntrinsic, {numSub, ctx()->constInt(1, 0, false)}, "clz");
        auto invClz = b.CreateSub(ctx()->constInt(64, 64, false), clz, "invclz");
        auto shlClz = b.CreateShl(ctx()->constInt(64, 1, false), invClz, "shlclz");
        auto reallocSize = b.CreateIntCast(shlClz, dataLayoutType, false, "reallocsize");
        reallocSize = b.CreateMul(
            reallocSize,
            ctx()->sizeOf(channelType)
        );
        auto newSamplePtr = b.CreateBitCast(CreateCall(b, reallocFunction, {
            b.CreateBitCast(samplePtr, voidPtrType, "sample.voidptr"), reallocSize
        }, "realloc.ptr"), llvm::PointerType::get(channelType, 0));
        b.CreateStore(shlClz, currentSizePtr);
        b.CreateStore(newSamplePtr, samplePtrPtr);
        b.CreateBr(resizeContinueBlock);
        b.SetInsertPoint(resizeContinueBlock);

        b.CreateRet(b.CreateLoad(resultPtr));
    }

    auto inputVal = dynamic_cast<Num *>(params[0].get());
    auto delayVal = dynamic_cast<Num *>(params[1].get());
    auto reserveVal = dynamic_cast<Num *>(params[2].get());

    auto leftPosPtr = method->getEntryPointer(addEntry(sizeType), "leftpos.ptr");
    auto rightPosPtr = method->getEntryPointer(addEntry(sizeType), "rightpos.ptr");
    auto leftBufferLengthPtr = method->getEntryPointer(addEntry(sizeType), "leftbuflength.ptr");
    auto rightBufferLengthPtr = method->getEntryPointer(addEntry(sizeType), "rightbuflength.ptr");

    auto leftSamplesPtrIndex = addEntry(llvm::PointerType::get(channelType, 0));
    auto rightSamplePtrIndex = addEntry(llvm::PointerType::get(channelType, 0));
    auto leftSamplesPtrPtr = method->getEntryPointer(leftSamplesPtrIndex, "leftsamples.ptr.ptr");
    auto rightSamplesPtrPtr = method->getEntryPointer(rightSamplePtrIndex, "rightsamples.ptr.ptr");

    // generate destructors for the two pointers
    auto &db = destructor()->builder();
    CreateCall(db, freeFunction, {db.CreateBitCast(
        db.CreateLoad(cdestructor()->getEntryPointer(leftSamplesPtrIndex, "leftsamples.ptr.ptr")), voidPtrType)}, "");
    CreateCall(db, freeFunction, {db.CreateBitCast(
        db.CreateLoad(cdestructor()->getEntryPointer(rightSamplePtrIndex, "rightsamples.ptr.ptr")), voidPtrType)}, "");

    auto &b = method->builder();
    auto inputVec = inputVal->vec(b);
    auto inputActive = inputVal->active(b);

    // determine reserve samples
    auto reserveSamplesUnclamped = b.CreateFMul(reserveVal->vec(b), ctx()->constFloatVec(ctx()->sampleRate),
                                                "reservesamples.unclamped");
    auto reserveSamplesFloat = CreateCall(b, maxIntrinsic, {reserveSamplesUnclamped, ctx()->constFloatVec(0)},
                                          "reservesamples.clamped");
    auto reserveSamples = b.CreateFPToUI(reserveSamplesFloat, llvm::VectorType::get(sizeType, 2), "reservesamples.int");

    // determine delay length with saturate(delayVal * reserve samples)
    auto delayValClamped = CreateCall(
        b, maxIntrinsic,
        {CreateCall(
            b, minIntrinsic,
            {delayVal->vec(b), ctx()->constFloatVec(1)},
            "delayval.clamped"
        ), ctx()->constFloatVec(0)},
        "delayval.clamped"
    );
    auto delaySamplesFloat = b.CreateFMul(delayValClamped, reserveSamplesFloat, "delaysamples.float");
    auto delaySamples = b.CreateFPToUI(delaySamplesFloat, llvm::VectorType::get(sizeType, 2), "delaysamples.int");

    // update the buffer
    auto leftResult = CreateCall(b, channelUpdateFunc, {
        leftPosPtr,
        leftBufferLengthPtr,
        b.CreateExtractElement(delaySamples, (uint64_t) 0),
        b.CreateExtractElement(reserveSamples, (uint64_t) 0),
        leftSamplesPtrPtr,
        inputActive,
        b.CreateExtractElement(inputVec, (uint64_t) 0)
    }, "result.left");
    auto rightResult = CreateCall(b, channelUpdateFunc, {
        rightPosPtr,
        rightBufferLengthPtr,
        b.CreateExtractElement(delaySamples, (uint64_t) 1),
        b.CreateExtractElement(reserveSamples, (uint64_t) 1),
        rightSamplesPtrPtr,
        inputActive,
        b.CreateExtractElement(inputVec, (uint64_t) 1)
    }, "result.right");

    auto resultNum = Num::create(ctx(), method->allocaBuilder());
    auto resultVec = b.CreateInsertElement(
        llvm::UndefValue::get(ctx()->numType()->vecType()),
        b.CreateExtractValue(leftResult, {1}, "num.left"),
        (uint64_t) 0, "result.vec"
    );
    resultVec = b.CreateInsertElement(resultVec, b.CreateExtractValue(rightResult, {1}, "num.right"), 1, "result.vec");
    resultNum->setVec(b, resultVec);

    auto resultActive = b.CreateOr(
        b.CreateExtractValue(leftResult, {0}, "active.left"),
        b.CreateExtractValue(rightResult, {0}, "active.right"),
        "result.active"
    );
    resultNum->setActive(b, resultActive);

    return std::move(resultNum);
}

std::vector<std::unique_ptr<Value>> DelayFunction::mapArguments(ComposableModuleClassMethod *method,
                                                                std::vector<std::unique_ptr<MaximCodegen::Value>> providedArgs) {
    if (providedArgs.size() < 3) {
        providedArgs.insert(providedArgs.begin() + 1,
                            Num::create(ctx(), method->allocaBuilder(), 1, 1, MaximCommon::FormType::LINEAR, true));
    }
    return providedArgs;
}