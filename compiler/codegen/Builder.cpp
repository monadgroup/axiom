#include "Builder.h"

using namespace MaximCodegen;

Builder::Builder(llvm::BasicBlock *block) : _builder(block) {

}

Builder::Builder(llvm::LLVMContext &context) : _builder(context) {

}

void Builder::SetInsertPoint(llvm::BasicBlock *block) { _builder.SetInsertPoint(block); }

llvm::GetElementPtrInst* Builder::CreateGEP(llvm::Type *type, llvm::Value *ptr, llvm::ArrayRef<llvm::Value *> idxList,
                                            const llvm::Twine &name) {
    if (auto constPtr = llvm::dyn_cast<llvm::Constant>(ptr)) {

    }
}
