#include "Runtime.h"

#include "ControlGroup.h"
#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Function.h"

using namespace MaximRuntime;

static const std::string initFuncName = "init";
static const std::string generateFuncName = "generate";
static const std::string globalCtxName = "globalCtx";
static const std::string outputName = "output";

Runtime::Runtime() : _context(_jit.dataLayout()), _op(&_context), _mainSchematic(this),
                     _module("controller", _context.llvm()) {
    auto libModule = std::make_unique<llvm::Module>("lib", _context.llvm());
    libModule->setDataLayout(_jit.dataLayout());
    _context.buildFunctions(libModule.get());
    _jit.addModule(std::move(libModule));
}

void Runtime::compileAndDeploy() {
    lock();

    if (_mainSchematic.needsCompile()) {
        _mainSchematic.compile();
        _mainSchematic.updateGetter(_mainSchematic.module());
    }
    if (_mainSchematic.needsDeploy()) _mainSchematic.deploy();

    auto mainFunc = _mainSchematic.inst();

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
    if (_isDeployed) _jit.removeModule(_deployKey);
    _deployKey = _jit.addModule(_module);
    _isDeployed = true;

    // update pointers for things that need to be accessed
    auto initFuncPtr = (void (*)()) _jit.getSymbolAddress(initFunc);
    _generateFuncPtr = (void (*)()) _jit.getSymbolAddress(generateFunc);
    assert(initFuncPtr);
    assert(_generateFuncPtr);

    // run the damn thing!
    initFuncPtr();

    // note: address of ctxGlobal _can_ be null, if it's empty - this isn't a problem, since there should be
    // nothing trying to read it
    _mainSchematic.updateCurrentPtr((void *) _jit.getSymbolAddress(ctxGlobal));

    unlock();
}

void Runtime::generate() {
    if (_generateFuncPtr) _generateFuncPtr();
}

void Runtime::fillBuffer(float **buffer, size_t size) {
    lock();

    auto outputPtr = _mainSchematic.output.control()->group()->currentPtr();
    assert(outputPtr);

    for (size_t i = 0; i < size; i++) {
        generate();
        auto outputNum = _op.readNum(outputPtr);
        buffer[0][i] = outputNum.left;
        buffer[1][i] = outputNum.right;
    }

    unlock();
}
