#include "Runtime.h"

#include "ControlGroup.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

static const std::string initFuncName = "init";
static const std::string generateFuncName = "generate";
static const std::string globalCtxName = "globalCtx";

Runtime::Runtime() : _context(jit.dataLayout()), _mainSchematic(this, nullptr, 0), _module("controller", _context.llvm()) {
    auto libModule = std::make_unique<llvm::Module>("lib", _context.llvm());
    libModule->setDataLayout(jit.dataLayout());
    _context.buildFunctions(libModule.get());
    jit.addModule(std::move(libModule));
}

void Runtime::compileAndDeploy() {
    if (_mainSchematic.needsCompile()) _mainSchematic.compile();
    if (_mainSchematic.needsDeploy()) _mainSchematic.deploy();

    auto mainFunc = _mainSchematic.instFunc();

    // remove old functions and global variables
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
        _module, mainFunc->type(&_context), false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        mainFunc->getInitialVal(&_context), globalCtxName
    );

    // create func to call any initializers
    auto initFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, initFuncName, &_module
    );
    auto initBlock = llvm::BasicBlock::Create(_context.llvm(), "entry", initFunc);
    MaximCodegen::Builder b(initBlock);
    b.CreateCall(mainFunc->initializeFunc(&_module), {ctxGlobal});
    b.CreateRetVoid();

    // create func to call on every generate cycle
    auto generateFunc = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {}),
        llvm::Function::LinkageTypes::ExternalLinkage, generateFuncName, &_module
    );
    auto generateBlock = llvm::BasicBlock::Create(_context.llvm(), "entry", generateFunc);
    b.SetInsertPoint(generateBlock);
    b.CreateCall(mainFunc->generateFunc(&_module), {ctxGlobal});
    b.CreateRetVoid();

    // deploy the new module to the JIT
    if (_isDeployed) jit.removeModule(_deployKey);
    _deployKey = jit.addModule(_module);
    _isDeployed = true;

    // update function pointers for the two functions we just made
    auto initFuncPtr = (void (*) ()) jit.getSymbolAddress(initFunc);
    _generateFuncPtr = (void (*) ()) jit.getSymbolAddress(generateFunc);

    // run the damn thing!
    initFuncPtr();
}

void Runtime::generate() {
    _generateFuncPtr();
}
