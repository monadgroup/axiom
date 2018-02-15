#pragma once

#include "../Function.h"
#include "../Instantiable.h"

namespace MaximCodegen {

    class HoldFunction : public Function {
    public:
        explicit HoldFunction(MaximContext *context);

        static std::unique_ptr<HoldFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;

        std::unique_ptr<Instantiable> generateCall(std::vector<std::unique_ptr<Value>> args) override;

    private:
        class FunctionCall : public Instantiable {
        public:
            llvm::Constant *getInitialVal(MaximContext *ctx) override;
            llvm::Type *type(MaximContext *ctx) const override;
        };

        static llvm::StructType *getContextType(MaximContext *ctx);
    };

}
