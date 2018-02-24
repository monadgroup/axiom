#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SawOscFunction : public PeriodicFunction {
    public:
        explicit SawOscFunction(MaximContext *context);

        static std::unique_ptr<SawOscFunction> create(MaximContext *context);

    protected:
        llvm::Value *nextValue(llvm::Value *period, Builder &b, llvm::Module *module) override;
    };

}
