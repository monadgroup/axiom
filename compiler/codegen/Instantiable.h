#pragma once

#include "Builder.h"

namespace llvm {
    class Constant;
    class Type;
}

namespace MaximCodegen {

    class MaximContext;

    class InstantiableFunction;

    class Instantiable {
    public:
        Instantiable();

        size_t id() const { return _id; }

        virtual llvm::Constant *getInitialVal(MaximContext *ctx) = 0;
        virtual void initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, InstantiableFunction *parent, Builder &b) {}
        virtual llvm::Type *type(MaximContext *ctx) const = 0;

    private:
        static size_t _nextId;
        size_t _id;
    };

}
