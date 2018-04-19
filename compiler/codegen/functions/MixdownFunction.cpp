#include "MixdownFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"
#include "../Array.h"
#include "../../util.h"

using namespace MaximCodegen;

MixdownFunction::MixdownFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "mixdown", ctx->numType(), {Parameter(ctx->getArrayType(ctx->numType()), true, false)},
               nullptr) {

}

std::unique_ptr<MixdownFunction> MixdownFunction::create(MaximContext *context, llvm::Module *module) {
    return std::make_unique<MixdownFunction>(context, module);
}

std::unique_ptr<Value> MixdownFunction::generate(ComposableModuleClassMethod *method,
                                                 const std::vector<std::unique_ptr<Value>> &params,
                                                 std::unique_ptr<VarArg> vararg) {
    auto inputArray = dynamic_cast<Array *>(params[0].get());
    assert(inputArray);

    auto &b = method->builder();
    auto result = Num::create(ctx(), inputArray->atIndex((size_t) 0, method->builder())->get(), method->builder(),
                              method->allocaBuilder());

    auto indexPtr = method->allocaBuilder().CreateAlloca(llvm::Type::getInt8Ty(ctx()->llvm()), nullptr, "index.ptr");
    b.CreateStore(ctx()->constInt(8, 1, false), indexPtr);

    auto func = method->get(method->moduleClass()->module());
    auto loopCheckBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopcheck", func);
    auto loopRunBlock = llvm::BasicBlock::Create(ctx()->llvm(), "looprun", func);
    auto loopEndBlock = llvm::BasicBlock::Create(ctx()->llvm(), "loopend", func);

    b.CreateBr(loopCheckBlock);
    b.SetInsertPoint(loopCheckBlock);

    auto currentIndex = b.CreateLoad(indexPtr, "index");
    auto indexCond = b.CreateICmpULT(currentIndex, ctx()->constInt(8, ArrayType::arraySize, false), "indexcond");
    b.CreateCondBr(indexCond, loopRunBlock, loopEndBlock);

    b.SetInsertPoint(loopRunBlock);
    auto nextIndex = b.CreateAdd(currentIndex, ctx()->constInt(8, 1, false), "nextindex");
    b.CreateStore(nextIndex, indexPtr);

    auto addNum = AxiomUtil::strict_unique_cast<Num>(inputArray->atIndex(currentIndex, b));
    result->setVec(b, b.CreateFAdd(result->vec(b), addNum->vec(b), "addvec"));
    result->setActive(b, b.CreateOr(result->active(b), addNum->active(b), "addactive"));
    b.CreateBr(loopCheckBlock);

    b.SetInsertPoint(loopEndBlock);
    return std::move(result);
}
