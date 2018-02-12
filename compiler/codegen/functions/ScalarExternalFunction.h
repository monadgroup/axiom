#pragma once
#include "../Function.h"

namespace MaximCodegen {

    class ScalarExternalFunction : public Function {
    public:
        ScalarExternalFunction(MaximContext *context, std::string externalName, std::string name, size_t paramCount, bool propagateForm, llvm::Module *module);

        static std::unique_ptr<ScalarExternalFunction> create(MaximContext *context, std::string externalName, std::string name, size_t paramCount, bool propagateForm, llvm::Module *module);

    protected:
        std::unique_ptr<Value> generate(Builder &b, std::vector<std::unique_ptr<Value>> params, std::unique_ptr<VarArg> vararg, llvm::Value *context) override;

    private:
        std::string externalName;
        bool propagateForm;
    };

}
