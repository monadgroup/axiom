#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class VectorScopeControl : public Control {
    public:
        VectorScopeControl(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<VectorScopeControl> create(MaximContext *ctx, llvm::Module *module);
    };

}
