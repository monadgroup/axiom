#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class VectorShuffleFunction : public Function {
    public:
        VectorShuffleFunction(MaximContext *ctx, llvm::Module *module, std::string name,
                              llvm::ArrayRef<uint32_t> shuffle);

        static std::unique_ptr<VectorShuffleFunction>
        create(MaximContext *ctx, llvm::Module *module, std::string name, llvm::ArrayRef<uint32_t> shuffle);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

    private:
        llvm::ArrayRef<uint32_t> _shuffle;
    };

}
