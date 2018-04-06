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

        llvm::Constant *initializeVal();

        virtual llvm::Type *storageType() = 0;

        virtual ModuleClassMethod *constructor() { return nullptr; }

        virtual ModuleClassMethod *destructor() { return nullptr; }

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

    class BasicModuleClass : public ModuleClass {
    public:
        BasicModuleClass(MaximContext *ctx, llvm::Module *module, const std::string &name, llvm::Type *type);

        void setStorageType(llvm::Type *type) { _storageType = type; }

        llvm::Type *storageType() { return _storageType; }

    private:
        llvm::Type *_storageType;
    };

}
