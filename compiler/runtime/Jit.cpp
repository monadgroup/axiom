#include "Jit.h"

#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/IR/Verifier.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Linker/Linker.h>
#include <iostream>

using namespace MaximRuntime;

llvm::JITSymbol getSymbol(const std::string &name) {
    // this doesn't seem to portably find math functions
    if (auto symAddr = llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name)) {
        return llvm::JITSymbol(symAddr, llvm::JITSymbolFlags::Exported);
    }

    // if the name starts with an underscore, try looking it up without
    if (name[0] == '_') {
        return getSymbol(name.substr(1));
    }

    if (name == "log2f") {
        return llvm::JITSymbol((uint64_t) &::log2f, llvm::JITSymbolFlags::Exported);
    } else if (name == "logbf") {
        return llvm::JITSymbol((uint64_t) &::logbf, llvm::JITSymbolFlags::Exported);
    } else if (name == "acosf") {
        return llvm::JITSymbol((uint64_t) &::acosf, llvm::JITSymbolFlags::Exported);
    } else if (name == "asinf") {
        return llvm::JITSymbol((uint64_t) &::asinf, llvm::JITSymbolFlags::Exported);
    } else if (name == "atan2f") {
        return llvm::JITSymbol((uint64_t) &::atan2f, llvm::JITSymbolFlags::Exported);
    } else if (name == "atanf") {
        return llvm::JITSymbol((uint64_t) &::atanf, llvm::JITSymbolFlags::Exported);
    } else if (name == "ceilf") {
        return llvm::JITSymbol((uint64_t) &::ceilf, llvm::JITSymbolFlags::Exported);
    } else if (name == "cosf") {
        return llvm::JITSymbol((uint64_t) &::cosf, llvm::JITSymbolFlags::Exported);
    } else if (name == "floorf") {
        return llvm::JITSymbol((uint64_t) &::floorf, llvm::JITSymbolFlags::Exported);
    } else if (name == "hypotf") {
        return llvm::JITSymbol((uint64_t) &::hypotf, llvm::JITSymbolFlags::Exported);
    } else if (name == "log10f") {
        return llvm::JITSymbol((uint64_t) &::log10f, llvm::JITSymbolFlags::Exported);
    } else if (name == "logf") {
        return llvm::JITSymbol((uint64_t) &::logf, llvm::JITSymbolFlags::Exported);
    } else if (name == "powf") {
        return llvm::JITSymbol((uint64_t) &::powf, llvm::JITSymbolFlags::Exported);
    } else if (name == "rand") {
        return llvm::JITSymbol((uint64_t) &::rand, llvm::JITSymbolFlags::Exported);
    } else if (name == "sinf") {
        return llvm::JITSymbol((uint64_t) &::sinf, llvm::JITSymbolFlags::Exported);
    } else if (name == "tanf") {
        return llvm::JITSymbol((uint64_t) &::tanf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fmaxf") {
        return llvm::JITSymbol((uint64_t) &::fmaxf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fminf") {
        return llvm::JITSymbol((uint64_t) &::fminf, llvm::JITSymbolFlags::Exported);
    } else if (name == "fmodf") {
        return llvm::JITSymbol((uint64_t) &::fmodf, llvm::JITSymbolFlags::Exported);
    } else if (name == "free") {
        return llvm::JITSymbol((uint64_t) &::free, llvm::JITSymbolFlags::Exported);
    }

    return nullptr;
}

Jit::Jit()
    : _targetMachine(llvm::EngineBuilder().selectTarget()),
      _dataLayout(_targetMachine->createDataLayout()),
      objectLayer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
      compileLayer(objectLayer, llvm::orc::SimpleCompiler(*_targetMachine)),
      optimizeLayer(compileLayer, [this](std::shared_ptr<llvm::Module> m) {
          return optimizeModule(std::move(m));
      }) {
    _targetMachine->Options.UnsafeFPMath = 1;
    _targetMachine->Options.NoTrappingFPMath = 1;
    _targetMachine->Options.NoSignedZerosFPMath = 1;
    _targetMachine->Options.FPDenormalMode = llvm::FPDenormal::PositiveZero;
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Jit::ModuleKey Jit::addModule(std::unique_ptr<llvm::Module> m) {
    if (linker) {
        auto success = !linker->linkInModule(llvm::CloneModule(m.get()));
        assert(success);
    }

    auto resolver = llvm::orc::createLambdaResolver(
        [&](const std::string &name) {
            if (auto sym = optimizeLayer.findSymbol(name, false)) return sym;
            return llvm::JITSymbol(nullptr);
        },
        [](const std::string &name) {
            return getSymbol(name);
        }
    );

    return std::move(llvm::cantFail(optimizeLayer.addModule(std::move(m), std::move(resolver))));
}

Jit::ModuleKey Jit::addModule(const llvm::Module &m) {
    return addModule(llvm::CloneModule(&m));
}

void Jit::removeModule(ModuleKey k) {
    llvm::cantFail(optimizeLayer.removeModule(k));
}

void Jit::markForRemove(MaximRuntime::Jit::ModuleKey k) {
    removeQueue.push_back(std::move(k));
}

void Jit::flushRemoveQueue() {
    while (!removeQueue.empty()) {
        removeModule(removeQueue.front());
        removeQueue.pop_front();
    }
}

llvm::JITSymbol Jit::findSymbol(const std::string &name) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    llvm::Mangler::getNameWithPrefix(mangledNameStream, name, _dataLayout);
    return optimizeLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
}

llvm::JITSymbol Jit::findSymbol(llvm::GlobalValue *value) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    mangler.getNameWithPrefix(mangledNameStream, value, false);
    return optimizeLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
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

void Jit::optimizeModule(llvm::Module *m, llvm::TargetMachine *targetMachine, unsigned int optLevel,
                         unsigned int sizeLevel) {
    llvm::legacy::PassManager mpm;
    llvm::legacy::FunctionPassManager fpm(m);

    fpm.add(llvm::createVerifierPass());
    fpm.add(llvm::createTargetTransformInfoWrapperPass(targetMachine->getTargetIRAnalysis()));

    llvm::PassManagerBuilder builder;
    builder.OptLevel = optLevel;
    builder.SizeLevel = sizeLevel;
    builder.Inliner = llvm::createFunctionInliningPass(builder.OptLevel, builder.SizeLevel, false);
    builder.LoopVectorize = true;
    builder.SLPVectorize = true;
    targetMachine->adjustPassManager(builder);
    builder.populateFunctionPassManager(fpm);
    builder.populateModulePassManager(mpm);

    fpm.doInitialization();
    for (auto &f : *m) {
        fpm.run(f);
    }
    mpm.run(*m);
}

std::shared_ptr<llvm::Module> Jit::optimizeModule(std::shared_ptr<llvm::Module> m) {
    optimizeModule(m.get(), _targetMachine, 3, 0);
    return m;
}
