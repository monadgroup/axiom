#pragma once

#include <memory>

namespace llvm {
    class Module;
    class Function;
}

namespace MaximCodegen {
    class ModuleClass;
}

namespace MaximRuntime {

    class Runtime;

    class RuntimeUnit {
    public:
        explicit RuntimeUnit(Runtime *runtime);

        Runtime *runtime() const { return _runtime; }

        virtual llvm::Module *module() = 0;

        /*void updateGetter(llvm::Module *module);

        void *getValuePtr(void *parentCtx);

        void *updateCurrentPtr(void *parentCtx);

        void *currentPtr() const;*/

    private:
        Runtime *_runtime;

        /*llvm::Function *getValueFunction = nullptr;

        void *(*_getValuePtr)(void *) = nullptr;

        void *_currentPtr = nullptr;*/
    };

}
