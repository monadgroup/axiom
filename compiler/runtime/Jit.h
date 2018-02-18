#pragma once

#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/IR/Mangler.h>

namespace llvm {
    class Module;
}

namespace MaximRuntime {

    class Jit {
    public:
        using ModuleKey = llvm::orc::VModuleKey;

        Jit();

        llvm::DataLayout const &dataLayout() const { return _dataLayout; }

        ModuleKey addModule(std::unique_ptr<llvm::Module> m);

        ModuleKey addModule(const llvm::Module &m);

        void removeModule(ModuleKey k);

        llvm::JITSymbol findSymbol(const std::string &name);

        llvm::JITSymbol findSymbol(llvm::GlobalValue *value);

        llvm::JITTargetAddress getSymbolAddress(const std::string &name);

        llvm::JITTargetAddress getSymbolAddress(llvm::GlobalValue *value);

    private:
        llvm::TargetMachine *targetMachine;
        const llvm::DataLayout _dataLayout;
        llvm::orc::SymbolStringPool symbolPool;
        llvm::orc::ExecutionSession executionSession;
        std::shared_ptr<llvm::orc::SymbolResolver> resolver;
        llvm::orc::RTDyldObjectLinkingLayer objectLayer;
        llvm::orc::IRCompileLayer<decltype(objectLayer), llvm::orc::SimpleCompiler> compileLayer;
        llvm::Mangler mangler;
    };

}
