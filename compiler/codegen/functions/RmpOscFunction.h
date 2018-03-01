#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class RmpOscFunction : public PeriodicFunction {
    public:
        explicit RmpOscFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<RmpOscFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) override;
    };

}
