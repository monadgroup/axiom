#pragma once

#include <memory>

namespace llvm {
    class Module;
}

namespace MaximCodegen {
    class ModuleClass;
}

namespace MaximRuntime {

    class Runtime;

    class RuntimeUnit {
    public:
        RuntimeUnit(Runtime *runtime, llvm::Module *module);

        Runtime *runtime() const { return _runtime; }

        llvm::Module *module() { return _module; }

        void updateGetter(llvm::Module *module);

        MaximCodegen::ModuleClass *compile();

        bool needsCompile() const { return _needsCompile; }

        void scheduleCompile();

        void *getValuePtr(void *parentCtx);

        void *updateCurrentPtr(void *parentCtx);

        void *currentPtr() const;

    protected:

        virtual MaximCodegen::ModuleClass *doCompile() = 0;

    private:
        Runtime *_runtime;
        llvm::Module *_module;
        bool _needsCompile = false;

        void *(*_getValuePtr)(void *) = nullptr;

        void *_currentPtr = nullptr;
    };

}
