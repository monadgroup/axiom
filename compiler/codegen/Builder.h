#pragma once

#include <llvm/IR/IRBuilder.h>

namespace MaximCodegen {

    using Builder = llvm::IRBuilder<>;

    llvm::Value *CreateCall(Builder &b, llvm::Function *f, llvm::ArrayRef<llvm::Value *> operands, const llvm::Twine &name);

}
