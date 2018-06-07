#pragma once

#include <llvm/IR/Module.h>

#include "RuntimeUnit.h"
#include "Jit.h"

namespace MaximRuntime {

    class ModuleRuntimeUnit : public RuntimeUnit {
    public:
        explicit ModuleRuntimeUnit(Runtime *runtime, const std::string &name);

        ~ModuleRuntimeUnit() override;

        llvm::Module *module() override { return _module.get(); }

        static std::unique_ptr<llvm::Module> createModule(const std::string &name, Runtime *runtime);

    protected:

        std::unique_ptr<llvm::Module> reset();

        std::unique_ptr<llvm::Module> setModule(std::unique_ptr<llvm::Module> module);

        void deploy();

    private:
        std::unique_ptr<llvm::Module> _module;

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;
    };

}
