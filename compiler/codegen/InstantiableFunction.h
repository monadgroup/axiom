#pragma once

#include "Instantiable.h"

namespace MaximCodegen {

    class InstantiableFunction : public Instantiable {
    public:
        InstantiableFunction(MaximContext *ctx, llvm::Module *module);

        static std::unique_ptr<InstantiableFunction> create(MaximContext *ctx, llvm::Module *module);

        MaximContext *ctx() const { return _ctx; }

        Builder &builder() { return _builder; }

        Builder &initBuilder() { return _initBuilder; }

        llvm::Module *module() const { return _module; }

        llvm::Function *generateFunc(llvm::Module *module);

        llvm::Function *initializeFunc(llvm::Module *module);

        llvm::Value *addInstantiable(std::unique_ptr<Instantiable> inst);

        llvm::Value *addInstantiable(Instantiable *inst);

        llvm::Value *getInitializePointer(Instantiable *inst);

        llvm::Value *getGeneratePointer(Instantiable *inst);

        virtual void complete();

        virtual void reset();

        llvm::Constant *getInitialVal(MaximContext *ctx) override;

        void initializeVal(MaximContext *ctx, llvm::Module *module, llvm::Value *ptr, InstantiableFunction *func,
                           Builder &b) override;

        llvm::StructType *type(MaximContext *ctx) const override;

        std::vector<Instantiable *> &instantiables() { return _instantiables; }

        bool getInstIndex(Instantiable *inst, std::vector<llvm::Value *> &indexes);

    private:
        MaximContext *_ctx;
        Builder _builder;
        Builder _initBuilder;
        llvm::Module *_module;

        llvm::Function *_generateFunc = nullptr;
        llvm::Function *_initializeFunc = nullptr;
        std::vector<llvm::Type *> _instTypes;

        std::vector<std::unique_ptr<Instantiable>> _ownedInstantiables;
        std::vector<Instantiable *> _instantiables;
    };

}
