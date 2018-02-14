#pragma once

#include "../Function.h"
#include "../Instantiable.h"

namespace MaximCodegen {

    class DelayFunction : public Function {
    public:
        explicit DelayFunction(MaximContext *context);

        static std::unique_ptr<DelayFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;

        std::unique_ptr<Instantiable> generateCall(std::vector<std::unique_ptr<Value>> args) override;

    private:
        class FunctionCall : public Instantiable {
        public:
            uint64_t leftDelaySize;
            uint64_t rightDelaySize;
            FunctionCall(uint64_t leftDelaySize, uint64_t rightDelaySize);
            llvm::Constant *getInitialVal(MaximContext *ctx) override;
            void initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) override;
            llvm::Type *type(MaximContext *ctx) const override;
        };

        static llvm::StructType *getChannelType(MaximContext *ctx);
        static llvm::StructType *getContextType(MaximContext *ctx);
    };

}
