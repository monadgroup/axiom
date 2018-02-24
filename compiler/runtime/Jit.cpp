#include "Jit.h"

#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <iostream>

using namespace MaximRuntime;

llvm::JITSymbol getSymbol(std::string name) {
    // this doesn't seem to portably find math functions
    if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
        return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
    }

    if (name == "log2f") {
        return llvm::JITSymbol((uint64_t)&::log2f, llvm::JITSymbolFlags::Exported);
    } else if (name == "logbf") {
        return llvm::JITSymbol((uint64_t)&::logbf, llvm::JITSymbolFlags::Exported);
    } else if (name == "acosf") {
        return llvm::JITSymbol((uint64_t)&::acosf, llvm::JITSymbolFlags::Exported);
    } else if (name == "asinf") {
        return llvm::JITSymbol((uint64_t)&::asinf, llvm::JITSymbolFlags::Exported);
    } else if (name == "atan2f") {
        return llvm::JITSymbol((uint64_t)&::atan2f, llvm::JITSymbolFlags::Exported);
    } else if (name == "atanf") {
        return llvm::JITSymbol((uint64_t)&::atanf, llvm::JITSymbolFlags::Exported);
    } else if (name == "ceilf") {
        return llvm::JITSymbol((uint64_t)&::ceilf, llvm::JITSymbolFlags::Exported);
    } else if (name == "cosf") {
        return llvm::JITSymbol((uint64_t)&::cosf, llvm::JITSymbolFlags::Exported);
    } else if (name == "floorf") {
        return llvm::JITSymbol((uint64_t)&::floorf, llvm::JITSymbolFlags::Exported);
    } else if (name == "hypotf") {
        return llvm::JITSymbol((uint64_t)&::hypotf, llvm::JITSymbolFlags::Exported);
    } else if (name == "log10f") {
        return llvm::JITSymbol((uint64_t)&::log10f, llvm::JITSymbolFlags::Exported);
    } else if (name == "logf") {
        return llvm::JITSymbol((uint64_t)&::logf, llvm::JITSymbolFlags::Exported);
    } else if (name == "powf") {
        return llvm::JITSymbol((uint64_t)&::powf, llvm::JITSymbolFlags::Exported);
    } else if (name == "rand") {
        return llvm::JITSymbol((uint64_t)&::rand, llvm::JITSymbolFlags::Exported);
    } else if (name == "sinf") {
        return llvm::JITSymbol((uint64_t)&::sinf, llvm::JITSymbolFlags::Exported);
    } else if (name == "tanf") {
        return llvm::JITSymbol((uint64_t)&::tanf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fmaxf") {
        return llvm::JITSymbol((uint64_t)&::fmaxf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fminf") {
        return llvm::JITSymbol((uint64_t)&::fminf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fmodf") {
        return llvm::JITSymbol((uint64_t)&::fmodf, llvm::JITSymbolFlags::Exported);
    }

    return nullptr;
}

Jit::Jit()
    : targetMachine(llvm::EngineBuilder().selectTarget()),
      _dataLayout(targetMachine->createDataLayout()),
      executionSession(symbolPool),
      resolver(llvm::orc::createLegacyLookupResolver(
          [this](const std::string &name) -> llvm::JITSymbol {
              if (auto sym = compileLayer.findSymbol(name, false)) return sym;
              else if (auto err = sym.takeError()) return std::move(err);

              if (auto sym = getSymbol(name)) return sym;
              if (name.length() > 2 && name[0] == '_') {
                  if (auto sym = getSymbol(name.substr(1))) return sym;
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
    _debugNames.emplace(k, m->getName().str());
    std::cout << "Adding module " << m->getName().str() << std::endl;

    auto result = compileLayer.addModule(k, std::move(m));
    return k;
}

Jit::ModuleKey Jit::addModule(const llvm::Module &m) {
    return addModule(llvm::CloneModule(m));
}

void Jit::removeModule(ModuleKey k) {
    auto keyIndex = _debugNames.find(k);
    assert(keyIndex != _debugNames.end());
    std::cout << "Removing module " << keyIndex->second << std::endl;

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
    auto addr = findSymbol(value).getAddress();
    if (auto err = addr.takeError()) {
        llvm::logAllUnhandledErrors(std::move(err), llvm::errs(), "");
        assert(false);
        throw;
    }

    return llvm::cantFail(std::move(addr));
}
