#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class ScalarControl : public Control {
    public:
        ScalarControl(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType, const std::string &name);

        static std::unique_ptr<ScalarControl> create(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType, const std::string &name);
    };

}
