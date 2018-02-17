#pragma once

#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/IR/Mangler.h>
#include <functional>

#include "../codegen/MaximContext.h"

namespace llvm {
    class TargetMachine;
}

namespace MaximRuntime {

    class Surface;

    class Runtime {
    private:
        using ObjectLayerTy = llvm::orc::RTDyldObjectLinkingLayer;
        using CompileLayerTy = llvm::orc::IRCompileLayer<ObjectLayerTy, llvm::orc::SimpleCompiler>;
    public:
        using ModuleHandle = llvm::orc::VModuleKey;

        llvm::TargetMachine *targetMachine;
        const llvm::DataLayout dataLayout;
        MaximCodegen::MaximContext context;
        std::unique_ptr<Surface> rootSurface;

        Runtime();

        void rebuild();

        void generate();

        ModuleHandle addModule(std::unique_ptr<llvm::Module> m);

        void removeModule(ModuleHandle h);

        llvm::JITSymbol findSymbol(const std::string &name);

        llvm::JITSymbol findSymbol(llvm::GlobalValue *value);

        llvm::JITTargetAddress getSymbolAddress(const std::string &name);

        llvm::JITTargetAddress getSymbolAddress(llvm::GlobalValue *value);

    private:
        llvm::orc::SymbolStringPool symbolPool;
        llvm::orc::ExecutionSession executionSession;
        std::shared_ptr<llvm::orc::SymbolResolver> resolver;
        llvm::orc::RTDyldObjectLinkingLayer objectLayer;
        llvm::orc::IRCompileLayer<decltype(objectLayer), llvm::orc::SimpleCompiler> compileLayer;
        llvm::Mangler mangler;

        llvm::Module _controllerModule;

        bool _hasHandle = false;
        ModuleHandle _handle;

        void (*_generateFunc)();
    };

}
