#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SawOscFunction : public PeriodicFunction {
    public:
        explicit SawOscFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<SawOscFunction> create(MaximContext *ctx, llvm::Module *module);

    protected:
        llvm::Value *nextValue(ComposableModuleClassMethod *method, llvm::Value *period) override;
    };

}
