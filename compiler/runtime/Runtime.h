#pragma once

#include <memory>
#include <vector>

#include "Jit.h"
#include "../codegen/MaximContext.h"
#include "Schematic.h"

namespace MaximRuntime {

    class ControlGroup;

    class Runtime {
    public:
        Jit jit;

        Runtime();

        MaximCodegen::MaximContext *context() { return &_context; }

        Schematic &mainSchematic() { return _mainSchematic; }

        void compileAndDeploy();

        void generate();

    private:
        MaximCodegen::MaximContext _context;
        Schematic _mainSchematic;
        llvm::Module _module;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;

        void (*_generateFuncPtr)() = nullptr;
    };

}
