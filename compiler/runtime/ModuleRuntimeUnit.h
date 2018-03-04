#pragma once

#include <llvm/IR/Module.h>

#include "RuntimeUnit.h"
#include "Jit.h"

namespace MaximRuntime {

    class ModuleRuntimeUnit : public RuntimeUnit {
    public:
        explicit ModuleRuntimeUnit(Runtime *runtime, const std::string &name);

    protected:

        void reset();

        void deploy();

    private:
        llvm::Module _module;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;
    };

}
