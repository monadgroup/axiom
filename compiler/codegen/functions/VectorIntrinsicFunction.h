#pragma once

#include <llvm/IR/Intrinsics.h>

#include "../Function.h"

namespace MaximCodegen {

    class VectorIntrinsicFunction : public Function {
    public:
        VectorIntrinsicFunction(MaximContext *ctx, llvm::Module *module, llvm::Intrinsic::ID id, std::string name,
                                size_t paramCount);

        static std::unique_ptr<VectorIntrinsicFunction>
        create(MaximContext *ctx, llvm::Module *module, llvm::Intrinsic::ID id, std::string name, size_t paramCount);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

    private:
        llvm::Intrinsic::ID id;
    };

}
