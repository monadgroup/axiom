#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SinOscFunction : public PeriodicFunction {
    public:
        explicit SinOscFunction(MaximContext *context);

        static std::unique_ptr<SinOscFunction> create(MaximContext *context);

    protected:
        llvm::Value *nextValue(llvm::Value *period, Builder &b, llvm::Module *module) override;
    };

}
