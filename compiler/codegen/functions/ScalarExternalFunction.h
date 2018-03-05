#pragma once

#include "../Function.h"

namespace MaximCodegen {

    class ScalarExternalFunction : public Function {
    public:
        ScalarExternalFunction(MaximContext *ctx, llvm::Module *module, std::string externalName, std::string name,
                               size_t paramCount);

        static std::unique_ptr<ScalarExternalFunction>
        create(MaximContext *ctx, llvm::Module *module, std::string externalName, std::string name, size_t paramCount);

    protected:
        std::unique_ptr<Value>
        generate(ComposableModuleClassMethod *method, const std::vector<std::unique_ptr<Value>> &params,
                 std::unique_ptr<VarArg> vararg) override;

    private:
        std::string _externalName;

    };

}
