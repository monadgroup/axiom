#include "Runtime.h"

#include "GeneratableModuleClass.h"
#include "ControlGroup.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Control.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

static const std::string initFuncName = "init";
static const std::string generateFuncName = "generate";
static const std::string globalCtxName = "globalCtx";

Runtime::Runtime() : _context(_jit.dataLayout()), _op(&_context),
                     _module("controller", _context.llvm()) {
    auto libModule = std::make_unique<llvm::Module>("lib", _context.llvm());
    libModule->setDataLayout(_jit.dataLayout());
    _context.setLibModule(libModule.get());
    _jit.addModule(std::move(libModule));

    // this must go after `setLibModule` as compilation reads from there
    _mainSurface = std::make_unique<RootSurface>(this);
}

void Runtime::compile() {
    lock();

    auto mainClass = _mainSurface->compile();

    if (auto oldInitFunc = _module.getFunction(initFuncName)) {
        oldInitFunc->removeFromParent();
    }
    if (auto oldGenerateFunc = _module.getFunction(generateFuncName)) {
        oldGenerateFunc->removeFromParent();
    }
    if (auto oldGlobalCtx = _module.getGlobalVariable(globalCtxName)) {
        oldGlobalCtx->removeFromParent();
    }

    // create global variable for all storage
    auto ctxGlobal = new llvm::GlobalVariable(
        _module, mainClass->storageType(), false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        mainClass->initializeVal(), globalCtxName
    );

    // create funcs to call the root surface
    // note: we could just call the surface methods and pass in the context,
    //       but this could break in case ModuleClassMethod ever changes how
    //       it passes in arguments
    auto initFunc = createForwardFunc("init", ctxGlobal, mainClass->constructor());
    auto generateFunc = createForwardFunc("generate", ctxGlobal, mainClass->generate());

    // deploy the new module to the JIT
    if (_isDeployed) _jit.removeModule(_deployKey);
    _deployKey = _jit.addModule(_module);
    _isDeployed = true;

    // update pointers for things that need to be accessed
    auto initFuncPtr = (void (*)()) _jit.getSymbolAddress(initFunc);
    _generateFuncPtr = (void (*)()) _jit.getSymbolAddress(generateFunc);
    assert(initFuncPtr && _generateFuncPtr);

    // initialize it
    initFuncPtr();

    // todo: handle accessors through the chain

    unlock();
}

void Runtime::generate() {
    if (_generateFuncPtr) _generateFuncPtr();
}

void Runtime::fillBuffer(float **buffer, size_t size) {
    lock();

    // todo

    unlock();
}

llvm::Function* Runtime::createForwardFunc(std::string name, llvm::Value *ctx,
                                           MaximCodegen::ModuleClassMethod *method) {
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, name, &_module
    );
    auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", func);
    MaximCodegen::Builder b(block);
    method->call(b, {}, ctx, &_module, "");
    b.CreateRetVoid();
    return func;
}
