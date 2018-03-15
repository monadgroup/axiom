#pragma once

#include "../Control.h"

namespace MaximCodegen {

    class ExtractControl : public Control {
    public:
        ExtractControl(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType, const std::string &name);

        static std::unique_ptr<ExtractControl>
        create(MaximContext *ctx, llvm::Module *module, MaximCommon::ControlType type, Type *storageType,
               const std::string &name);
    };

}
