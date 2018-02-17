#include "Runtime.h"

#include <llvm/Transforms/Utils/Cloning.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/ExecutionEngine/Orc/LambdaResolver.h>

#include "Node.h"
#include "Surface.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

Runtime::Runtime()
    : targetMachine(llvm::EngineBuilder().selectTarget()),
      dataLayout(targetMachine->createDataLayout()),
      context(dataLayout), rootSurface(std::make_unique<Surface>(this)),
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
      compileLayer(objectLayer, llvm::orc::SimpleCompiler(*targetMachine)),
      _controllerModule("controller", context.llvm()) {
    _controllerModule.setDataLayout(dataLayout);

    auto mainModule = std::make_unique<llvm::Module>("main", context.llvm());
    mainModule->setDataLayout(dataLayout);
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

    auto func = rootSurface->getFunction();

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
        llvm::Function::LinkageTypes::ExternalLinkage, "__init", &_controllerModule
    );
    auto initBlock = llvm::BasicBlock::Create(context.llvm(), "entry", initFunc);
    MaximCodegen::Builder b(initBlock);
    b.CreateCall(func->initializeFunc(&_controllerModule), {ctxGlobal});
    b.CreateRetVoid();

    auto generateFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, "__generate", &_controllerModule
    );
    auto generateBlock = llvm::BasicBlock::Create(context.llvm(), "entry", generateFunc);
    b.SetInsertPoint(generateBlock);
    b.CreateCall(func->generateFunc(&_controllerModule), {ctxGlobal});
    b.CreateRetVoid();

    if (_hasHandle) removeModule(_handle);
    _handle = addModule(llvm::CloneModule(_controllerModule));
    _hasHandle = true;

    auto initFuncPtr = (void (*)()) getSymbolAddress(initFunc);
    _generateFunc = (void (*)()) getSymbolAddress(generateFunc);

    initFuncPtr();
}

void Runtime::generate() {
    if (_generateFunc) _generateFunc();
}

Runtime::ModuleHandle Runtime::addModule(std::unique_ptr<llvm::Module> m) {
    auto k = executionSession.allocateVModule();
    auto result = compileLayer.addModule(k, std::move(m));
    return k;
}

void Runtime::removeModule(ModuleHandle h) {
    llvm::cantFail(compileLayer.removeModule(h));
}

llvm::JITSymbol Runtime::findSymbol(const std::string &name) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    llvm::Mangler::getNameWithPrefix(mangledNameStream, name, dataLayout);
    return compileLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
}

llvm::JITSymbol Runtime::findSymbol(llvm::GlobalValue *value) {
    std::string mangledName;
    llvm::raw_string_ostream mangledNameStream(mangledName);
    mangler.getNameWithPrefix(mangledNameStream, value, false);
    return compileLayer.findSymbol(mangledNameStream.str(), false); // todo: shouldn't need false here
}

llvm::JITTargetAddress Runtime::getSymbolAddress(const std::string &name) {
    return llvm::cantFail(findSymbol(name).getAddress());
}

llvm::JITTargetAddress Runtime::getSymbolAddress(llvm::GlobalValue *value) {
    return llvm::cantFail(findSymbol(value).getAddress());
}
