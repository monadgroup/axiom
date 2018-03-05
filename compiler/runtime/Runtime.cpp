#include <iostream>
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

Runtime::Runtime() : _context(_jit.dataLayout()), _op(&_context) {
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

    auto module = std::make_unique<llvm::Module>("controller", _context.llvm());

    // create global variable for all storage
    auto ctxGlobal = new llvm::GlobalVariable(
        *module, mainClass->storageType(), false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        mainClass->initializeVal(), globalCtxName
    );

    // create funcs to call the root surface
    // note: we could just call the surface methods and pass in the context,
    //       but this could break in case ModuleClassMethod ever changes how
    //       it passes in arguments
    createForwardFunc(module.get(), initFuncName, ctxGlobal, mainClass->constructor());
    createForwardFunc(module.get(), generateFuncName, ctxGlobal, mainClass->generate());

    // deploy the new module to the JIT
    if (_isDeployed) _jit.removeModule(_deployKey);
    _deployKey = _jit.addModule(std::move(module));
    _isDeployed = true;

    // update pointers for things that need to be accessed
    auto ctxGlobalPtr = (void *) _jit.getSymbolAddress(globalCtxName);
    auto initFuncPtr = (void (*)()) _jit.getSymbolAddress(initFuncName);
    _generateFuncPtr = (void (*)()) _jit.getSymbolAddress(generateFuncName);
    assert(ctxGlobalPtr && initFuncPtr && _generateFuncPtr);

    // propagate new pointers
    _mainSurface->updateCurrentPtr(ctxGlobalPtr);

    // initialize it
    initFuncPtr();

    unlock();
}

void Runtime::generate() {
    if (_generateFuncPtr) _generateFuncPtr();
}

void Runtime::fillBuffer(float **buffer, size_t size) {
    lock();

    auto outputPtr = _mainSurface->output.control()->group()->currentPtr();
    assert(outputPtr);

    for (size_t i = 0; i < size; i++) {
        generate();
        auto outputNum = _op.readNum(outputPtr);
        buffer[0][i] = outputNum.left;
        buffer[1][i] = outputNum.right;
    }

    unlock();
}

llvm::Function* Runtime::createForwardFunc(llvm::Module *module, std::string name, llvm::Value *ctx,
                                           MaximCodegen::ModuleClassMethod *method) {
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, name, module
    );
    auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", func);
    MaximCodegen::Builder b(block);
    method->call(b, {}, ctx, module, "");
    b.CreateRetVoid();
    return func;
}
