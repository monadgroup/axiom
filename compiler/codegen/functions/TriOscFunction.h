#pragma once

#include "PeriodicFunction.h"

namespace MaximCodegen {

    class TriOscFunction : public PeriodicFunction {
    public:
        explicit TriOscFunction(MaximContext *context);

        static std::unique_ptr<TriOscFunction> create(MaximContext *context);

    protected:
        llvm::Value *nextValue(llvm::Value *period, Builder &b, llvm::Module *module) override;
    };

}
