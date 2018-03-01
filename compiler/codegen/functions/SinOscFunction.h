#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SinOscFunction : public PeriodicFunction {
    public:
        explicit SinOscFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SinOscFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) override;
    };

}
