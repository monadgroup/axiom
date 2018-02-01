#pragma once

#include <llvm/IR/IRBuilder.h>

namespace MaximCodegen {

    class FormType;
    class MidiType;
    class NumType;
    class TupleType;

    class Context {
    public:
        llvm::LLVMContext &llvm() { return _llvm; }
        llvm::IRBuilder<> &builder() { return _builder; }

    private:
        llvm::LLVMContext _llvm;
        llvm::IRBuilder<> _builder;
    };

}
