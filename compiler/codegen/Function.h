#pragma once

namespace llvm {
    class BasicBlock;
}

namespace MaximCodegen {

    class Scope;
    class Context;

    class Function {
    public:
        Context *context() const;
        Scope *scope() const;

        llvm::BasicBlock *initBlock() const { return _initBlock; }
        llvm::BasicBlock *codeBlock() const { return _codeBlock; }

        llvm::IRBuilder<> &initBuilder() { return _initBuilder; }
        llvm::IRBuilder<> &codeBuilder() { return _codeBuilder; }

    private:
        llvm::BasicBlock *_initBlock;
        llvm::BasicBlock *_codeBlock;
        llvm::IRBuilder<> _initBuilder;
        llvm::IRBuilder<> _codeBuilder;
    };

}
