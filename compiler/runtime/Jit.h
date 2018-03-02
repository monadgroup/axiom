#pragma once

#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/IR/Mangler.h>

#include <unordered_map>

namespace llvm {
    class Module;
}

namespace MaximRuntime {

    class Jit {
    private:
        using ObjectLayer = llvm::orc::RTDyldObjectLinkingLayer;
        using CompileLayer = llvm::orc::IRCompileLayer<ObjectLayer, llvm::orc::SimpleCompiler>;

    public:
        using ModuleKey = CompileLayer::ModuleHandleT;

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
        ObjectLayer objectLayer;
        CompileLayer compileLayer;
        llvm::Mangler mangler;
    };

}
