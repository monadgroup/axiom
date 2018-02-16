#pragma once

#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
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
        using ModuleHandle = CompileLayerTy::ModuleHandleT;

        std::unique_ptr<Surface> rootSurface;
        MaximCodegen::MaximContext context;

        Runtime();

        void rebuild();

        void generate();

        ModuleHandle addModule(std::unique_ptr<llvm::Module> m);

        void removeModule(ModuleHandle h);

        llvm::JITSymbol findSymbol(const std::string &name);

        llvm::JITTargetAddress getSymbolAddress(const std::string &name);

    private:
        std::unique_ptr<llvm::TargetMachine> targetMachine;
        const llvm::DataLayout dataLayout;
        ObjectLayerTy objectLayer;
        CompileLayerTy compileLayer;
        llvm::Module _controllerModule;

        bool _hasHandle;
        ModuleHandle _handle;

        void (*_generateFunc)();
    };

}
