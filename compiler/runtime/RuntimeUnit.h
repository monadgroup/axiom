#pragma once

#include <memory>

namespace llvm {
    class Module;

    class Function;

    class Type;
}

namespace MaximCodegen {
    class ModuleClass;

    class ModuleClassMethod;
}

namespace MaximRuntime {

    class Runtime;

    class RuntimeUnit {
    public:
        explicit RuntimeUnit(Runtime *runtime);

        virtual ~RuntimeUnit();

        Runtime *runtime() const { return _runtime; }

        virtual llvm::Module *module() = 0;

        void setMethods(std::unique_ptr<MaximCodegen::ModuleClassMethod> getterMethod, MaximCodegen::ModuleClassMethod *destructorMethod);

        MaximCodegen::ModuleClassMethod *getterMethod() const { return _getterMethod.get(); }

        MaximCodegen::ModuleClassMethod *destructorMethod() const { return _destructorMethod; }

        virtual void pullMethods();

        virtual void *getValuePtr(void *parentCtx);

        virtual void *updateCurrentPtr(void *parentCtx);

        virtual void saveValue();

        virtual void restoreValue();

        void destructIfNeeded();

        void cleanup();

        void *currentPtr() const;

        virtual MaximCodegen::ModuleClass *moduleClass() = 0;

    private:
        struct ValueDeleter {
            void operator()(void *x) { free(x); }
        };

        Runtime *_runtime;

        std::unique_ptr<MaximCodegen::ModuleClassMethod> _getterMethod;
        MaximCodegen::ModuleClassMethod *_destructorMethod = nullptr;

        using GetValueCb = void *(*)(void *);
        using DestructCb = void (*)(void *);

        GetValueCb _getValueCb = nullptr;
        DestructCb _destructCb = nullptr;

        void *_currentPtr = nullptr;

        llvm::Type *_saveType;
        std::unique_ptr<void, ValueDeleter> _saveValue;
    };

}
