#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFunction : public Function {
    public:
        VectorIntrinsicFunction(MaximContext *context, llvm::Intrinsic::ID id, std::string name, size_t paramCount, bool propagateForm, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *context) override;

    private:
        llvm::Intrinsic::ID id;
        bool propagateForm;
    };

}
