#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class TriOscFunction : public PeriodicFunction {
    public:
        explicit TriOscFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<TriOscFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) override;
    };

}
