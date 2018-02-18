#pragma once

#include <llvm/IR/Module.h>

#include "../codegen/InstantiableFunction.h"
#include "Jit.h"

namespace MaximRuntime {

    class Runtime;

    class CompileUnit {
    public:
        explicit CompileUnit(Runtime *runtime);

        ~CompileUnit();

        Runtime *runtime() const { return _runtime; }

        virtual CompileUnit *parentUnit() const = 0;

        llvm::Module *module() { return &_module; }

        virtual MaximCodegen::InstantiableFunction *instFunc() { return &_instFunc; }

        bool needsCompile() const { return _needsCompile; }

        bool needsDeploy() const { return _needsDeploy; }

        bool isDeployed() const { return _isDeployed; }

        void scheduleCompile();

        void scheduleDeploy();

        virtual void compile();

        virtual void deploy();

    protected:

        void cancelDeploy() { _needsDeploy = false; }

    private:
        Runtime *_runtime;
        llvm::Module _module;
        MaximCodegen::InstantiableFunction _instFunc;
        bool _needsCompile = true;
        bool _needsDeploy = false;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;
    };

}
