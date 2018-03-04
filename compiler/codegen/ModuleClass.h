#pragma once

#include "Builder.h"
#include "ModuleClassMethod.h"

namespace MaximCodegen {

    class ModuleClassMethod;

    class MaximContext;

    class ModuleClass {
    public:
        ModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name);

        size_t index() const { return _index; }

        MaximContext *ctx() const { return _ctx; }

        llvm::Module *module() const { return _module; }

        const std::string &name() const { return _name; }

        virtual llvm::Constant *initializeVal() = 0;

        virtual llvm::Type *storageType() = 0;

        virtual ModuleClassMethod *constructor() = 0;

        bool completed() const { return _completed; }

        void complete();

        std::string mangleMethodName(const std::string &name);

    protected:

        virtual void doComplete();

    private:
        static size_t _nextIndex;
        size_t _index;

        MaximContext *_ctx;
        llvm::Module *_module;
        std::string _name;
        bool _completed = false;
    };

    class ZeroInitializedModuleClass : public ModuleClass {
    public:
        ZeroInitializedModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name);

        llvm::Constant *initializeVal() override;
    };

    class UndefInitializedModuleClass : public ModuleClass {
    public:
        UndefInitializedModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name);

        llvm::Constant *initializeVal() override;
    };

    class TypeInferencedModuleClass : public ModuleClass {
    public:
        TypeInferencedModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name);

        llvm::Type *storageType() override;
    };

}
