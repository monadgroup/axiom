#pragma once

#include <llvm/IR/Module.h>

#include "../codegen/InstantiableFunction.h"
#include "Jit.h"
#include "CompileLeaf.h"

namespace MaximRuntime {

    class Runtime;

    class CompileUnit : public CompileLeaf {
    public:
        explicit CompileUnit(Runtime *runtime);

        virtual ~CompileUnit();

        CompileUnit *parentUnit() const override = 0;

        llvm::Module *module() { return &_module; }

        MaximCodegen::InstantiableFunction *inst() override { return &_instFunc; }

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
        llvm::Module _module;
        MaximCodegen::InstantiableFunction _instFunc;
        bool _needsCompile = true;
        bool _needsDeploy = false;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;
    };

}
