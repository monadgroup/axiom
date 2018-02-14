#include "WithActiveFunction.h"

#include <llvm/IR/InstrTypes.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

WithActiveFunction::WithActiveFunction(MaximContext *context)
    : Function(context, "withActive", context->numType(), {Parameter(context->numType(), false, false), Parameter(context->numType(), false, false)}, nullptr, nullptr) {

}

std::unique_ptr<WithActiveFunction> WithActiveFunction::create(MaximContext *context) {
    return std::make_unique<WithActiveFunction>(context);
}

std::unique_ptr<Value> WithActiveFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                    std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                    llvm::Function *func, llvm::Module *module) {
    auto xNum = dynamic_cast<Num*>(params[0].get());
    auto activeNum = dynamic_cast<Num*>(params[1].get());
    assert(xNum && activeNum);

    auto activeVec = activeNum->vec(b);
    auto activeVal = b.CreateFCmp(
        llvm::CmpInst::Predicate::FCMP_ONE,
        b.CreateExtractElement(activeVec, (uint64_t) 0, "active.left"),
        context()->constFloat(0),
        "active.notzero"
    );

    auto undefPos = SourcePos(-1, -1);
    return xNum->withActive(b, activeVal, undefPos, undefPos);
}
