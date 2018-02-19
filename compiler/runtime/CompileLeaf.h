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

        void *getValuePtr(void *parentCtx);

    protected:

        llvm::Function *getValueFunction = nullptr;

    private:

        void *(*_getValuePtr)(void *) = nullptr;

        Runtime *_runtime;

    };

}
