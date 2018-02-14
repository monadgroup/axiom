#pragma once

#include "../Function.h"
#include "../Instantiable.h"

namespace MaximCodegen {

    class NextFunction : public Function {
    public:
        explicit NextFunction(MaximContext *context);

        static std::unique_ptr<NextFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *funcContext, llvm::Function *func, llvm::Module *module) override;

        std::unique_ptr<Instantiable> generateCall(std::vector<std::unique_ptr<Value>> args);

    private:
        class NextFunctionCall : public Instantiable {
        public:
            llvm::Constant *getInitialVal(MaximContext *ctx) override;
            llvm::Type *type(MaximContext *ctx) const override;
        };
    };

}
