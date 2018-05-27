#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class VectorScopeControl : public Control {
    public:
        // minimum fps of screen update, used to determine size of buffer
        static constexpr uint8_t minimumFps = 30;

        VectorScopeControl(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<VectorScopeControl> create(MaximContext *ctx, llvm::Module *module);
    };

}
