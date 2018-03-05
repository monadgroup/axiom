#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFoldFunction : public Function {
    public:
        VectorIntrinsicFoldFunction(MaximContext *ctx, llvm::Module *module, llvm::Intrinsic::ID id, std::string name);

        static std::unique_ptr<VectorIntrinsicFoldFunction>
        create(MaximContext *ctx, llvm::Module *module, llvm::Intrinsic::ID id, std::string name);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

        std::unique_ptr<Value>
        generateConst(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                      std::unique_ptr<ConstVarArg> vararg) override;

    private:
        llvm::Intrinsic::ID _id;
    };

}
