#pragma once

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/IRTransformLayer.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/Mangler.h>
#include <unordered_map>

namespace llvm {
    class Module;
}

class OrcJit {
private:
    using ObjectLayer = llvm::orc::RTDyldObjectLinkingLayer;
    using CompileLayer = llvm::orc::IRCompileLayer<ObjectLayer, llvm::orc::SimpleCompiler>;

public:
    explicit OrcJit(llvm::TargetMachine &targetMachine)
        : dataLayout(targetMachine.createDataLayout()),
          objectLayer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
          compileLayer(objectLayer, llvm::orc::SimpleCompiler(targetMachine)) {}

    using ModuleKey = unsigned;

    std::string mangle(llvm::StringRef name) {
        std::string mangledName;
        llvm::raw_string_ostream mangledNameStream(mangledName);
        llvm::Mangler::getNameWithPrefix(mangledNameStream, name, dataLayout);
        return mangledName;
    }

    void addBuiltin(llvm::StringRef name, llvm::JITTargetAddress address) { builtins.emplace(mangle(name), address); }

    ModuleKey addModule(std::shared_ptr<llvm::Module> module) {
        auto resolver = llvm::orc::createLambdaResolver(
            [&](const std::string &name) {
                if (auto sym = compileLayer.findSymbol(name, false)) return sym;
                return llvm::JITSymbol(nullptr);
            },
            [&](const std::string &name) {
                auto builtinAddress = builtins.find(name);
                if (builtinAddress != builtins.end()) {
                    return llvm::JITSymbol(builtinAddress->second, llvm::JITSymbolFlags::Exported);
                }
                return llvm::JITSymbol(nullptr);
            });

        auto handle = llvm::cantFail(compileLayer.addModule(std::move(module), std::move(resolver)));
        return createKey(std::move(handle));
    }

    void removeModule(ModuleKey k) {
        auto handle = std::move(genericHandles[k]);
        freeHandleIndexes.push_back(k);
        llvm::cantFail(compileLayer.removeModule(std::move(handle)));
    }

    llvm::JITSymbol findSymbol(llvm::StringRef name) { return compileLayer.findSymbol(mangle(name), false); }

    llvm::JITTargetAddress getSymbolAddress(llvm::StringRef name) {
        return llvm::cantFail(findSymbol(name).getAddress());
    }

private:
    llvm::DataLayout dataLayout;
    ObjectLayer objectLayer;
    CompileLayer compileLayer;
    std::unordered_map<std::string, llvm::JITTargetAddress> builtins;

    std::vector<CompileLayer::ModuleHandleT> genericHandles;
    std::vector<unsigned> freeHandleIndexes;

    unsigned createKey(CompileLayer::ModuleHandleT handle) {
        unsigned newHandle;
        if (!freeHandleIndexes.empty()) {
            newHandle = freeHandleIndexes.back();
            freeHandleIndexes.pop_back();
            genericHandles[newHandle] = std::move(handle);
        } else {
            newHandle = genericHandles.size();
            genericHandles.push_back(std::move(handle));
        }
        return newHandle;
    }
};
