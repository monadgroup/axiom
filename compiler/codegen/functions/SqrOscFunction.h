#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SqrOscFunction : public PeriodicFunction {
    public:
        explicit SqrOscFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SqrOscFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) override;
    };

}
