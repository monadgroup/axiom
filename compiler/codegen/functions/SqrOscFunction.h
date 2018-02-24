#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class SqrOscFunction : public PeriodicFunction {
    public:
        explicit SqrOscFunction(MaximContext *context);

        static std::unique_ptr<SqrOscFunction> create(MaximContext *context);

    protected:
        llvm::Value *nextValue(llvm::Value *period, Builder &b, llvm::Module *module) override;
    };

}
