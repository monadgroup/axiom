#pragma once

#include <memory>
#include <unordered_map>

#include "Instantiable.h"
#include "Builder.h"

namespace MaximAst {
    class AssignableExpression;
}

namespace MaximCodegen {

    class MaximContext;

    class Value;

    class Node : public Instantiable {
    public:
        explicit Node(MaximContext *ctx, llvm::Module *module);

        MaximContext *ctx() const { return _ctx; }

        Builder &builder() { return _builder; }

        llvm::Module *module() const { return _module; }

        llvm::Function *func() const { return _func; }

        Value *getVariable(std::string name);

        void setVariable(std::string name, std::unique_ptr<Value> value);

        void setAssignable(MaximAst::AssignableExpression *assignable, std::unique_ptr<Value> value);

        llvm::Value *addInstantiable(std::unique_ptr<Instantiable> inst, Builder &b);

        void complete();

        llvm::Constant *getInitialVal(MaximContext *ctx) override;

        void initializeVal(MaximContext *ctx, llvm::Value *ptr, Builder &b) override;

        llvm::Type *type(MaximContext *ctx) const override { return _ctxType; }

    private:
        MaximContext *_ctx;
        Builder _builder;
        llvm::Module *_module;

        llvm::Function *_func;
        llvm::Value *_nodeCtx;
        llvm::StructType *_ctxType;
        std::vector<llvm::Type*> _instTypes;

        std::unordered_map<std::string, std::unique_ptr<Value>> _variables;
        std::vector<std::unique_ptr<Instantiable>> _instantiables;
    };

}
