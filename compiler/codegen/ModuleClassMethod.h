#pragma once

#include "Builder.h"

namespace MaximCodegen {

    class ModuleClass;

    class ModuleClassMethod {
    public:
        ModuleClassMethod(ModuleClass *moduleClass, std::string name, llvm::Type *returnType = nullptr,
                          std::vector<llvm::Type *> paramTypes = {});

        virtual ModuleClass *moduleClass() const { return _moduleClass; }

        const std::string &name() const { return _name; }

        llvm::Function *get(llvm::Module *module) const;

        llvm::BasicBlock *entryBlock() { return _entryBlock; }

        Builder &builder() { return _builder; }

        virtual llvm::Value *contextPtr() const { return _contextPtr; }

        llvm::Value *arg(size_t index) const;

        llvm::Value *call(Builder &b, std::vector<llvm::Value *> args, llvm::Value *context, llvm::Module *module,
                          const llvm::Twine &resultName) const;

    private:
        ModuleClass *_moduleClass;
        std::string _name;
        llvm::BasicBlock *_entryBlock;
        Builder _builder;
        llvm::Type *_returnType;
        std::vector<llvm::Type *> _paramTypes;
        llvm::Value *_contextPtr;
    };

}
