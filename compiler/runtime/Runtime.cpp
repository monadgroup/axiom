#include "Runtime.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/IR/Mangler.h>

#include "Node.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

Runtime::Runtime()
    : rootSurface(&context),
      targetMachine(llvm::EngineBuilder().selectTarget()), dataLayout(targetMachine->createDataLayout()),
      objectLayer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
      compileLayer(objectLayer, llvm::orc::SimpleCompiler(*targetMachine)) {
    auto mainModule = std::make_unique<llvm::Module>("main", context.llvm());
    context.buildFunctions(mainModule.get());
    addModule(std::move(mainModule));
}

Runtime::ModuleHandle Runtime::addModule(std::unique_ptr<llvm::Module> m) {
    auto resolver = llvm::orc::createLambdaResolver(
        [&](const std::string &name) {
            if (auto sym = compileLayer.findSymbol(name, false)) return sym;
            return llvm::JITSymbol(nullptr);
        },
        [](const std::string &name) {
            if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
                return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
            }
            return llvm::JITSymbol(nullptr);
        }
    );

    return llvm::cantFail(compileLayer.addModule(std::move(m), std::move(resolver)));
}

void Runtime::removeModule(ModuleHandle h) {
    llvm::cantFail(compileLayer.removeModule(h));
}

llvm::JITSymbol Runtime::findSymbol(const std::string &name) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    llvm::Mangler::getNameWithPrefix(mangledNameStream, name, dataLayout);
    return compileLayer.findSymbol(mangledNameStream.str(), true);
}

llvm::JITTargetAddress Runtime::getSymbolAddress(const std::string &name) {
    return llvm::cantFail(findSymbol(name).getAddress());
}
