#pragma once

namespace llvm {

    class Module;

    class Function;

}

namespace MaximCodegen {

    class Instantiable;

}

namespace MaximRuntime {

    class CompileUnit;

    class Runtime;

    class CompileLeaf {
    public:

        explicit CompileLeaf(Runtime *runtime);

        virtual MaximCodegen::Instantiable *inst() = 0;

        virtual CompileUnit *parentUnit() const = 0;

        Runtime *runtime() const { return _runtime; }

        void updateGetter(llvm::Module *module);

        virtual void deploy();

        virtual void *getValuePtr(void *parentCtx);

        virtual void *updateCurrentPtr(void *parentCtx);

        void *currentPtr() const;

    protected:

        llvm::Function *getValueFunction = nullptr;

    private:

        void *(*_getValuePtr)(void *) = nullptr;

        void *_currentPtr = nullptr;

        Runtime *_runtime;

    };

}
