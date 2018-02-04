#pragma once

#include "llvm/IR/IRBuilder.h"
#include "Scope.h"

namespace MaximCodegen {

    class Context;

    class Function {
    public:
        Function(llvm::Function *fun, Context *context);

        Context *context() { return _context; }

        Scope *scope() { return &_scope; }

        llvm::BasicBlock *initBlock() const { return _initBlock; }

        llvm::BasicBlock *codeBlock() const { return _codeBlock; }

        llvm::IRBuilder<> &initBuilder() { return _initBuilder; }

        llvm::IRBuilder<> &codeBuilder() { return _codeBuilder; }

    private:
        Context *_context;
        Scope _scope;

        llvm::BasicBlock *_initBlock;
        llvm::BasicBlock *_codeBlock;
        llvm::IRBuilder<> _initBuilder;
        llvm::IRBuilder<> _codeBuilder;
    };

}
