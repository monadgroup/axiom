#pragma once

#include <memory>
#include <vector>

#include "Jit.h"
#include "../codegen/MaximContext.h"
#include "RootSchematic.h"

namespace MaximRuntime {

    class ControlGroup;

    class Runtime {
    public:
        Jit jit;

        Runtime();

        MaximCodegen::MaximContext *context() { return &_context; }

        RootSchematic &mainSchematic() { return _mainSchematic; }

        void compileAndDeploy();

        void generate();

        llvm::GlobalVariable *outputPtr(llvm::Module *module);

        void *output() const { return _outputPtr; }

        void *globalCtx() const { return _globalCtxPtr; }

    private:
        MaximCodegen::MaximContext _context;
        RootSchematic _mainSchematic;
        llvm::Module _module;
        llvm::GlobalVariable *_outputGlobal;
        void *_outputPtr = nullptr;
        void *_globalCtxPtr = nullptr;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;

        void (*_generateFuncPtr)() = nullptr;
    };

}
