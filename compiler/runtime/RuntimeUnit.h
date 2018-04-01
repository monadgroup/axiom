#pragma once

#include <memory>

namespace llvm {
    class Module;

    class Function;

    class Type;
}

namespace MaximCodegen {
    class ModuleClass;

    class ComposableModuleClassMethod;
}

namespace MaximRuntime {

    class Runtime;

    class RuntimeUnit {
    public:
        struct MallocDeleter {
            void operator()(void *x) { free(x); }
        };

        struct SaveValue {
            llvm::Type *type;
            std::unique_ptr<void, MallocDeleter> value;
        };

        explicit RuntimeUnit(Runtime *runtime);

        virtual ~RuntimeUnit();

        Runtime *runtime() const { return _runtime; }

        virtual llvm::Module *module() = 0;

        void setGetterMethod(std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> method);

        MaximCodegen::ComposableModuleClassMethod *getterMethod() const { return _method.get(); }

        virtual void pullGetterMethod(MaximCodegen::ComposableModuleClassMethod *method = nullptr);

        virtual void *getValuePtr(void *parentCtx);

        virtual void *updateCurrentPtr(void *parentCtx);

        virtual void saveValue();

        virtual void restoreValue();

        virtual void setRestoreValue(SaveValue &value);

        SaveValue moveValue() { return std::move(_saveVal); };

        void *currentPtr() const;

        virtual MaximCodegen::ModuleClass *moduleClass() = 0;

    private:
        Runtime *_runtime;

        std::unique_ptr<MaximCodegen::ComposableModuleClassMethod> _method;

        using GetValueCb = void *(*)(void *);

        GetValueCb _getValueCb = nullptr;

        void *_currentPtr = nullptr;

        SaveValue _saveVal = {
            nullptr,
            nullptr
        };
    };

}
