#pragma once

#include "../Function.h"
#include "../Instantiable.h"

namespace MaximCodegen {

    class DelayFunction : public Function {
    public:
        explicit DelayFunction(MaximContext *context);

        static std::unique_ptr<DelayFunction> create(MaximContext *context);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params, std::unique_ptr<VarArg> vararg) override;

        std::vector<std::unique_ptr<Value>> mapArguments(std::vector<std::unique_ptr<Value>> providedArgs) override;

    private:
        //static llvm::StructType *getChannelType(MaximContext *ctx);

        //static llvm::StructType *getContextType(MaximContext *ctx);
    };

}
