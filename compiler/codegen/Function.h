#pragma once

#include "llvm/IR/IRBuilder.h"
#include "Scope.h"
#include "FunctionDeclaration.h"

namespace MaximCodegen {

    class Context;

    class FunctionDeclaration;

    class Function {
    public:
        Function(FunctionDeclaration decl, llvm::Function *fun, Context *context);

        Context *context() { return _context; }

        Scope *scope() { return &_scope; }

        FunctionDeclaration *decl() { return &_decl; }

        llvm::Function *llFunc() const { return _llFunc; }

        llvm::BasicBlock *initBlock() const { return _initBlock; }

        llvm::BasicBlock *codeBlock() const { return _codeBlock; }

        llvm::IRBuilder<> &initBuilder() { return _initBuilder; }

        llvm::IRBuilder<> &codeBuilder() { return _codeBuilder; }

    private:
        Context *_context;
        Scope _scope;
        FunctionDeclaration _decl;

        llvm::Function *_llFunc;
        llvm::BasicBlock *_initBlock;
        llvm::BasicBlock *_codeBlock;
        llvm::IRBuilder<> _initBuilder;
        llvm::IRBuilder<> _codeBuilder;
    };

}
