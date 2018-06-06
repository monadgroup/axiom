#pragma once

#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/IR/Mangler.h>

#include <deque>

namespace llvm {
    class Module;

    class Linker;
}

namespace MaximRuntime {

    class Jit {
    private:
        using ObjectLayer = llvm::orc::RTDyldObjectLinkingLayer;
        using CompileLayer = llvm::orc::IRCompileLayer<ObjectLayer, llvm::orc::SimpleCompiler>;
        using OptimizeFunction = std::function<std::shared_ptr<llvm::Module>(std::shared_ptr<llvm::Module>)>;
        using OptimizeLayer = llvm::orc::IRTransformLayer<CompileLayer, OptimizeFunction>;

    public:
        using ModuleKey = CompileLayer::ModuleHandleT;

        llvm::Linker *linker = nullptr;

        Jit();

        llvm::TargetMachine *targetMachine() const { return _targetMachine; }

        llvm::DataLayout const &dataLayout() const { return _dataLayout; }

        ModuleKey addModule(std::unique_ptr<llvm::Module> m);

        ModuleKey addModule(const llvm::Module &m);

        void removeModule(ModuleKey k);

        void markForRemove(ModuleKey k);

        void flushRemoveQueue();

        llvm::JITSymbol findSymbol(const std::string &name);

        llvm::JITSymbol findSymbol(llvm::GlobalValue *value);

        llvm::JITTargetAddress getSymbolAddress(const std::string &name);

        llvm::JITTargetAddress getSymbolAddress(llvm::GlobalValue *value);

        std::shared_ptr<llvm::Module> optimizeModule(std::shared_ptr<llvm::Module> m);

        static void optimizeModule(llvm::Module *m, llvm::TargetMachine *targetMachine, unsigned int optLevel, unsigned int sizeLevel);

    private:
        llvm::TargetMachine *_targetMachine;
        const llvm::DataLayout _dataLayout;
        ObjectLayer objectLayer;
        CompileLayer compileLayer;
        OptimizeLayer optimizeLayer;
        llvm::Mangler mangler;

        std::deque<ModuleKey> removeQueue;
    };

}
