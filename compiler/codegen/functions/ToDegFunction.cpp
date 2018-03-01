#include "ToDegFunction.h"

#include "../MaximContext.h"
#include "../ComposableModuleClassMethod.h"
#include "../Num.h"

using namespace MaximCodegen;

ToDegFunction::ToDegFunction(MaximContext *ctx, llvm::Module *module)
    : Function(ctx, module, "toDeg", ctx->numType(), {Parameter(ctx->numType(), false, false)}, nullptr) {

}

std::unique_ptr<ToDegFunction> ToDegFunction::create(MaximContext *ctx, llvm::Module *module) {
    return std::make_unique<ToDegFunction>(ctx, module);
}

std::unique_ptr<Value> ToDegFunction::generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) {
    auto numArg = dynamic_cast<Num *>(params[0].get());
    assert(numArg);

    auto &b = method->builder();

    auto multConst = ctx()->constFloat(180.f / (float) M_PI);
    auto multVec = llvm::ConstantVector::get({multConst, multConst});
    auto newVec = b.CreateBinOp(
        llvm::Instruction::BinaryOps::FMul,
        numArg->vec(b),
        multVec,
        "mult"
    );

    auto undefPos = SourcePos(-1, -1);
    return numArg->withVec(b, newVec, undefPos, undefPos);
}
