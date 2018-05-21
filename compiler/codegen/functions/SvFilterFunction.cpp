#include "SvFilterFunction.h"

#define _USE_MATH_DEFINES

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"
#include "../Tuple.h"

using namespace MaximCodegen;

SvFilterFunction::SvFilterFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "svFilter",
               ctx->getTupleType({ctx->numType(), ctx->numType(), ctx->numType(), ctx->numType()}),
               {Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false),
                Parameter(ctx->numType(), false, false)},
               nullptr) {

}

std::unique_ptr<SvFilterFunction> SvFilterFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<SvFilterFunction>(ctx, module);
}

std::unique_ptr<Value> SvFilterFunction::generate(ComposableModuleClassMethod *method,
                                                  const std::vector<std::unique_ptr<Value>> &params,
                                                  std::unique_ptr<VarArg> vararg) {
    auto sinFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::sin,
                                                   {ctx()->numType()->vecType()});
    auto minFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::minnum,
                                                   {ctx()->numType()->vecType()});
    auto powFunc = llvm::Intrinsic::getDeclaration(method->moduleClass()->module(), llvm::Intrinsic::ID::pow,
                                                   {ctx()->numType()->vecType()});

    auto inputNum = dynamic_cast<Num *>(params[0].get());
    auto freqNum = dynamic_cast<Num *>(params[1].get());
    auto qNum = dynamic_cast<Num *>(params[2].get());
    assert(inputNum && freqNum && qNum);

    auto &b = method->builder();
    auto notchPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "notch.ptr");
    auto lowPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "low.ptr");
    auto highPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "high.ptr");
    auto bandPtr = method->getEntryPointer(method->moduleClass()->addEntry(ctx()->floatVecTy()), "band.ptr");

    auto inputVec = inputNum->vec(b);

    auto fParam = b.CreateFMul(
        freqNum->vec(b),
        b.CreateFDiv(ctx()->constFloatVec(M_PI), ctx()->constFloatVec(ctx()->sampleRate)),
        "fparam"
    );
    auto fOver2 = CreateCall(b, sinFunc, {fParam}, "fover2");
    auto fVal = b.CreateFMul(ctx()->constFloatVec(2), fOver2, "f");

    auto qVec = qNum->vec(b);
    auto qv = b.CreateFDiv(ctx()->constFloatVec(1), qVec);

    // calculate dampening factor
    auto twoQ = b.CreateFMul(qv, ctx()->constFloatVec(2), "twoq");
    auto twoQRecip = b.CreateFDiv(ctx()->constFloatVec(1), twoQ, "twoqrecip");
    auto dampPowBase = b.CreateFSub(ctx()->constFloatVec(1), twoQRecip, "damppowbase");
    auto dampPow = CreateCall(b, powFunc, {dampPowBase, ctx()->constFloatVec(0.25)}, "damppow");
    auto inverseDampPow = b.CreateFSub(ctx()->constFloatVec(1), dampPow, "inversedamppow");
    auto dampVal = b.CreateFMul(inverseDampPow, ctx()->constFloatVec(2), "dampval");

    auto maxDampLeft = b.CreateFDiv(ctx()->constFloatVec(2), fVal, "maxdamp.left");
    auto maxDampRight = b.CreateFMul(ctx()->constFloatVec(0.5), fVal, "maxdamp.right");
    auto maxDamp = b.CreateFSub(maxDampLeft, maxDampRight, "maxdamp");
    maxDamp = CreateCall(b, minFunc, {ctx()->constFloatVec(2), maxDamp}, "maxdamp");

    auto damp = CreateCall(b, minFunc, {dampVal, maxDamp}, "damp");

    auto loopIndexPtr = method->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(ctx()->llvm()), nullptr,
                                                             "index.ptr");
    b.CreateStore(ctx()->constInt(8, 0, false), loopIndexPtr);
    auto loopCount = ctx()->constInt(8, 2, false);

    auto func = method->get(method->moduleClass()->module());
    auto loopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopcheck", func);
    auto loopBodyBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopbody", func);
    auto loopEndBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopend", func);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopCheckBlock);

    auto currentIndex = b.CreateLoad(loopIndexPtr, "currentindex");
    auto indexCond = b.CreateICmpULT(currentIndex, loopCount, "indexcond");
    b.CreateCondBr(indexCond, loopBodyBlock, loopEndBlock);

    b.SetInsertPoint(loopBodyBlock);

    b.CreateStore(
        b.CreateFSub(inputVec, b.CreateFMul(damp, b.CreateLoad(bandPtr)), "newnotch"),
        notchPtr
    );
    b.CreateStore(
        b.CreateFAdd(b.CreateLoad(lowPtr), b.CreateFMul(fVal, b.CreateLoad(bandPtr)), "newlow"),
        lowPtr
    );
    b.CreateStore(
        b.CreateFSub(b.CreateLoad(notchPtr), b.CreateLoad(lowPtr), "newhigh"),
        highPtr
    );
    b.CreateStore(
        b.CreateFAdd(b.CreateFMul(fVal, b.CreateLoad(highPtr)), b.CreateLoad(bandPtr), "newband"),
        bandPtr
    );

    auto nextIndex = b.CreateAdd(currentIndex, ctx()->constInt(8, 1, false), "indexptr");
    b.CreateStore(nextIndex, loopIndexPtr);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopEndBlock);

    auto inputForm = inputNum->form(b);
    auto inputActive = inputNum->active(b);

    Tuple::Storage tupleVecs;
    auto resultHigh = Num::create(ctx(), method->allocaBuilder());
    resultHigh->setVec(b, b.CreateLoad(highPtr, "highvec"));
    resultHigh->setForm(b, inputForm);
    resultHigh->setActive(b, inputActive);
    tupleVecs.push_back(std::move(resultHigh));

    auto resultLow = Num::create(ctx(), method->allocaBuilder());
    resultLow->setVec(b, b.CreateLoad(lowPtr, "lowvec"));
    resultLow->setForm(b, inputForm);
    resultLow->setActive(b, inputActive);
    tupleVecs.push_back(std::move(resultLow));

    auto resultBand = Num::create(ctx(), method->allocaBuilder());
    resultBand->setVec(b, b.CreateLoad(bandPtr, "bandvec"));
    resultBand->setForm(b, inputForm);
    resultBand->setActive(b, inputActive);
    tupleVecs.push_back(std::move(resultBand));

    auto resultNotch = Num::create(ctx(), method->allocaBuilder());
    resultNotch->setVec(b, b.CreateLoad(notchPtr, "notchvec"));
    resultNotch->setForm(b, inputForm);
    resultNotch->setActive(b, inputActive);
    tupleVecs.push_back(std::move(resultNotch));

    auto undefPos = SourcePos(-1, -1);
    return Tuple::create(
        ctx(),
        std::move(tupleVecs),
        b, method->allocaBuilder(), undefPos, undefPos
    );
}
