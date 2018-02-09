#pragma once

#include "Scope.h"
#include "../SourcePos.h"
#include "values/Value.h"
#include "Builder.h"

namespace MaximCodegen {

    class Context;

    class FunctionDeclaration;

    class Parameter;

    class Function {
    public:
        struct ParamData {
            std::unique_ptr<Value> value;
            SourcePos start;
            SourcePos end;

            ParamData(std::unique_ptr<Value> value, SourcePos start, SourcePos end)
                    : value(std::move(value)), start(start), end(end) {}
        };

        Function(std::unique_ptr<FunctionDeclaration> decl, const std::string &name, llvm::Function::LinkageTypes linkage, llvm::Module *module,
                 Context *context);

        Context *context() { return _context; }

        Scope *scope() { return &_scope; }

        FunctionDeclaration *decl() { return _decl.get(); }

        llvm::Function *llFunc() const { return _llFunc; }

        llvm::BasicBlock *initBlock() const { return _initBlock; }

        llvm::BasicBlock *codeBlock() const { return _codeBlock; }

        Builder &initBuilder() { return _initBuilder; }

        Builder &codeBuilder() { return _codeBuilder; }

        std::unique_ptr<Value> generateCall(const std::vector<ParamData> &params, SourcePos start, SourcePos end, Function *function);

    private:
        Context *_context;
        Scope _scope;
        std::unique_ptr<FunctionDeclaration> _decl;

        llvm::Function *_llFunc;
        llvm::BasicBlock *_initBlock;
        llvm::BasicBlock *_codeBlock;
        Builder _initBuilder;
        Builder _codeBuilder;

        void checkParam(const ParamData *paramItem, const Parameter *paramData);
    };

}
