#pragma once

#include "../codegen/Instantiable.h"
#include "Control.h"

namespace MaximRuntime {

    class OutputNode;

    class OutputControl : public Control, public MaximCodegen::Instantiable {
    public:
        explicit OutputControl(OutputNode *node);

        MaximCommon::ControlDirection direction() const override { return MaximCommon::ControlDirection::IN; }

        MaximCodegen::Control *control() const override { return nullptr; }

        llvm::Constant *getInitialVal(MaximCodegen::MaximContext *ctx) override;

        llvm::Type *type(MaximCodegen::MaximContext *ctx) const override;
    };

}
