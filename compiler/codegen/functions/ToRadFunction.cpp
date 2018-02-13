#include "ToRadFunction.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

ToRadFunction::ToRadFunction(MaximContext *context)
    : Function(context, "toRad", context->numType(), {Parameter(context->numType(), false, false)}, nullptr, nullptr) {

}

std::unique_ptr<ToRadFunction> ToRadFunction::create(MaximContext *context) {
    return std::make_unique<ToRadFunction>(context);
}

std::unique_ptr<Value> ToRadFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                               llvm::Function *func, llvm::Module *module) {
    auto numArg = dynamic_cast<Num*>(params[0].get());
    assert(numArg);

    auto multConst = context()->constFloat((float)M_PI / 180.f);
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
