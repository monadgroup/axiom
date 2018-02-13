#include "VectorShuffleFunction.h"

#include <llvm/IR/Constants.h>

#include "../MaximContext.h"
#include "../Num.h"

using namespace MaximCodegen;

VectorShuffleFunction::VectorShuffleFunction(MaximContext *context, std::string name, llvm::ArrayRef<uint32_t> shuffle)
    : Function(context, std::move(name), context->numType(),
               {Parameter(context->numType(), false, false)}, nullptr, nullptr), _shuffle(shuffle) {

}

std::unique_ptr<VectorShuffleFunction> VectorShuffleFunction::create(MaximContext *context, std::string name,
                                                                     llvm::ArrayRef<uint32_t> shuffle) {
    return std::make_unique<VectorShuffleFunction>(context, name, shuffle);
}

std::unique_ptr<Value> VectorShuffleFunction::generate(Builder &b, std::vector<std::unique_ptr<Value>> params,
                                                       std::unique_ptr<VarArg> vararg, llvm::Value *funcContext,
                                                       llvm::Function *func, llvm::Module *module) {
    auto xNum = dynamic_cast<Num*>(params[0].get());
    assert(xNum);

    auto xVec = xNum->vec(b);
    auto newVec = b.CreateShuffleVector(
        xVec, llvm::UndefValue::get(context()->numType()->vecType()), _shuffle, "shuffled"
    );

    auto undefPos = SourcePos(-1, -1);
    return xNum->withVec(b, newVec, undefPos, undefPos);
}
