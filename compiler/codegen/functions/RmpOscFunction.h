#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class RmpOscFunction : public PeriodicFunction {
    public:
        explicit RmpOscFunction(MaximContext *context);

        static std::unique_ptr<RmpOscFunction> create(MaximContext *context);

    protected:
        llvm::Value *nextValue(llvm::Value *period, Builder &b, llvm::Module *module) override;
    };

}
