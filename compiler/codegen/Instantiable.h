#pragma once

#include "Builder.h"

namespace llvm {
    class Constant;
    class Type;
}

namespace MaximCodegen {

    class MaximContext;

    class Instantiable {
    public:
        virtual llvm::Constant *getInitialVal(MaximContext *ctx) = 0;
        virtual void initializeVal(MaximContext *ctx, llvm::Value *ptr, Builder &b) {}
        virtual llvm::Type *type(MaximContext *ctx) const = 0;
    };

}
