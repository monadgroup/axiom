#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class VectorShuffleFunction : public Function {
    public:
        VectorShuffleFunction(MaximContext *context, std::string name, llvm::ArrayRef<uint32_t> shuffle, llvm::Module *module);

        static std::unique_ptr<VectorShuffleFunction> create(MaximContext *context, std::string name, llvm::ArrayRef<uint32_t> shuffle, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext) override;

    private:
        llvm::ArrayRef<uint32_t> _shuffle;
    };

}
