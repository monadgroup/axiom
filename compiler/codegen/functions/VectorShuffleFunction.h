#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class VectorShuffleFunction : public Function {
    public:
        VectorShuffleFunction(MaximContext *context, std::string name, llvm::ArrayRef<uint32_t> shuffle);

        static std::unique_ptr<VectorShuffleFunction> create(MaximContext *context, std::string name, llvm::ArrayRef<uint32_t> shuffle);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

    private:
        llvm::ArrayRef<uint32_t> _shuffle;
    };

}
