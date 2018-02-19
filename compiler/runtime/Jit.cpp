#include "Jit.h"

#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>

using namespace MaximRuntime;

Jit::Jit()
    : targetMachine(llvm::EngineBuilder().selectTarget()),
      _dataLayout(targetMachine->createDataLayout()),
      executionSession(symbolPool),
      resolver(llvm::orc::createLegacyLookupResolver(
          [this](const std::string &name) -> llvm::JITSymbol {
              if (auto sym = compileLayer.findSymbol(name, false)) return sym;
              else if (auto err = sym.takeError()) return std::move(err);

              if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
                  return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
              }
              return nullptr;
          },
          [](llvm::Error err) { llvm::cantFail(std::move(err), "lookupFlags failed"); }
      )),
      objectLayer(executionSession, [this](llvm::orc::VModuleKey) {
          return llvm::orc::RTDyldObjectLinkingLayer::Resources {
              std::make_shared<llvm::SectionMemoryManager>(), resolver
          };
      }),
      compileLayer(objectLayer, llvm::orc::SimpleCompiler(*targetMachine)) {

}

Jit::ModuleKey Jit::addModule(std::unique_ptr<llvm::Module> m) {
    auto k = executionSession.allocateVModule();
    auto result = compileLayer.addModule(k, std::move(m));
    return k;
}

Jit::ModuleKey Jit::addModule(const llvm::Module &m) {
    return addModule(llvm::CloneModule(m));
}

void Jit::removeModule(ModuleKey k) {
    llvm::cantFail(compileLayer.removeModule(k));
}

llvm::JITSymbol Jit::findSymbol(const std::string &name) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    llvm::Mangler::getNameWithPrefix(mangledNameStream, name, _dataLayout);
    return compileLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
}

llvm::JITSymbol Jit::findSymbol(llvm::GlobalValue *value) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    mangler.getNameWithPrefix(mangledNameStream, value, false);
    return compileLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
}

llvm::JITTargetAddress Jit::getSymbolAddress(const std::string &name) {
    return llvm::cantFail(findSymbol(name).getAddress());
}

llvm::JITTargetAddress Jit::getSymbolAddress(llvm::GlobalValue *value) {
    return llvm::cantFail(findSymbol(value).getAddress());
}
