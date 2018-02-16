#include "Runtime.h"

#include <llvm/Transforms/Utils/Cloning.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>
#include <llvm/IR/Mangler.h>

#include "Node.h"
#include "Surface.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

Runtime::Runtime()
    : rootSurface(std::make_unique<Surface>(&context)),
      targetMachine(llvm::EngineBuilder().selectTarget()), dataLayout(targetMachine->createDataLayout()),
      objectLayer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
      compileLayer(objectLayer, llvm::orc::SimpleCompiler(*targetMachine)),
      _controllerModule("controller", context.llvm()) {
    auto mainModule = std::make_unique<llvm::Module>("main", context.llvm());
    context.buildFunctions(mainModule.get());
    addModule(std::move(mainModule));
}

void Runtime::rebuild() {
    if (!rootSurface->isDirty()) return;

    if (auto oldInitFunc = _controllerModule.getFunction("init")) {
        oldInitFunc->removeFromParent();
    }
    if (auto oldGenerateFunc = _controllerModule.getFunction("generate")) {
        oldGenerateFunc->removeFromParent();
    }

    auto func = rootSurface->getFunction(this);

    auto ctxGlobal = new llvm::GlobalVariable(
        _controllerModule,
        func->type(&context),
        false,
        llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        func->getInitialVal(&context),
        "ctx"
    );

    auto initFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, "init", &_controllerModule
    );
    auto initBlock = llvm::BasicBlock::Create(context.llvm(), "entry", initFunc);
    MaximCodegen::Builder b(initBlock);
    b.CreateCall(func->initializeFunc(&_controllerModule), {ctxGlobal});

    auto generateFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, "generate", &_controllerModule
    );
    auto generateBlock = llvm::BasicBlock::Create(context.llvm(), "entry", generateFunc);
    b.SetInsertPoint(generateBlock);
    b.CreateCall(func->generateFunc(&_controllerModule), {ctxGlobal});

    if (_hasHandle) removeModule(_handle);
    addModule(llvm::CloneModule(&_controllerModule));

    auto initFuncPtr = (void (*)()) getSymbolAddress("init");
    initFuncPtr();

    _generateFunc = (void (*)()) getSymbolAddress("generate");
}

void Runtime::generate() {
    if (_generateFunc) _generateFunc();
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
