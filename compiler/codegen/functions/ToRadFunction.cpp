#include "ToRadFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ToRadFunction::ToRadFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "toRad", ctx->numType(), {Parameter(ctx->numType(), false, false)}, nullptr) {

}

std::unique_ptr<ToRadFunction> ToRadFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ToRadFunction>(ctx, module);
}

std::unique_ptr<Value>
ToRadFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                        std::unique_ptr<VarArg> vararg) {
    auto numArg = dynamic_cast<Num *>(params[0].get());
    assert(numArg);

    auto &b = method->builder();

    auto multConst = ctx()->constFloat((float) M_PI / 180.f);
    auto multVec = llvm::ConstantVector::get({multConst, multConst});
    auto newVec = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        numArg->vec(b),
        multVec,
        "mult"
    );

    auto newNum = Num::create(ctx(), numArg->get(), b, method->allocaBuilder());
    newNum->setVec(b, newVec);
    return std::move(newNum);
}
