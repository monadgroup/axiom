#include "Function.h"

#include "Context.h"
#include "values/Value.h"
#include "Control.h"

using namespace MaximCodegen;

Function::Function(FunctionDeclaration decl, llvm::Function *fun, Context *context)
        : _context(context), _scope(_context), _decl(decl), _llFunc(fun), _initBuilder(context->llvm()),
          _codeBuilder(context->llvm()) {
    _initBlock = llvm::BasicBlock::Create(context->llvm(), "entry", fun);
    _initBuilder.SetInsertPoint(_initBlock);

    _codeBlock = llvm::BasicBlock::Create(context->llvm(), "code", fun);
    _codeBuilder.SetInsertPoint(_codeBlock);
}
