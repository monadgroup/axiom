#include "ToDegFunction.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

ToDegFunction::ToDegFunction(MaximContext *context)
    : Function(context, "toDeg", context->numType(), {Parameter(context->numType(), false, false)}, nullptr, nullptr) {

}

std::unique_ptr<ToDegFunction> ToDegFunction::create(MaximContext *context) {
    return std::make_unique<ToDegFunction>(context);
}

std::unique_ptr<Value> ToDegFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                               std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                               llvm::Module *module) {
    auto numArg = dynamic_cast<Num*>(params[0].get());
    assert(numArg);

    auto multConst = context()->constFloat(180.f / (float)M_PI);
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
