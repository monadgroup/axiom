#pragma once

#include "Instantiable.h"

namespace MaximCodegen {

    class InstantiableFunction : public Instantiable {
    public:
        explicit InstantiableFunction(MaximContext *ctx, llvm::Module *module);

        MaximContext *ctx() const { return _ctx; }

        Builder &builder() { return _builder; }

        llvm::Module *module() const { return _module; }

        llvm::Function *func() const { return _func; }

        llvm::Value *addInstantiable(std::unique_ptr<Instantiable> inst, Builder &b);

        void complete();

        llvm::Constant *getInitialVal(MaximContext *ctx) override;

        void initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, Builder &b) override;

        llvm::Type *type(MaximContext *ctx) const override { return _ctxType; }

    private:
        MaximContext *_ctx;
        Builder _builder;
        llvm::Module *_module;

        llvm::Function *_func;
        llvm::Value *_nodeCtx;
        llvm::StructType *_ctxType;
        std::vector<llvm::Type*> _instTypes;

        std::vector<std::unique_ptr<Instantiable>> _instantiables;
    };

}
