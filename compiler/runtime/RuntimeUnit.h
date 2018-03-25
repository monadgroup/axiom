#pragma once

#include <memory>

namespace llvm {
    class Module;

    class Function;
}

namespace MaximCodegen {
    class ModuleClass;

    class ComposableModuleClassMethod;
}

namespace MaximRuntime {

    class Runtime;

    class RuntimeUnit {
    public:
        explicit RuntimeUnit(Runtime *runtime);

        virtual ~RuntimeUnit();

        Runtime *runtime() const { return _runtime; }

        virtual llvm::Module *module() = 0;

        void setGetterMethod(std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> method);

        MaximCodegen::ComposableModuleClassMethod *getterMethod() const { return _method.get(); }

        virtual void pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method = nullptr);

        virtual void *getValuePtr(void *parentCtx);

        virtual void *updateCurrentPtr(void *parentCtx);

        void *currentPtr() const;

    private:
        Runtime *_runtime;

        std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> _method;

        using GetValueCb = void *(*)(void *);

        GetValueCb _getValueCb = nullptr;

        void *_currentPtr = nullptr;
    };

}
