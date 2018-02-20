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

Runtime::Runtime() : _context(jit.dataLayout()), _mainSchematic(this), _module("controller", _context.llvm()) {
    auto libModule = std::make_unique<llvm::Module>("lib", _context.llvm());
    libModule->setDataLayout(jit.dataLayout());
    _context.buildFunctions(libModule.get());

    _outputGlobal = new llvm::GlobalVariable(
        *libModule, _context.numType()->get(), false, llvm::GlobalVariable::ExternalLinkage,
        llvm::ConstantStruct::get(_context.numType()->get(), {
            llvm::ConstantVector::getSplat(2, _context.constFloat(0)),
            llvm::ConstantInt::get(_context.numType()->formType(), (uint64_t) MaximCommon::FormType::LINEAR, false),
            llvm::ConstantInt::get(_context.numType()->activeType(), (uint64_t) false, false)
        }), outputName
    );

    jit.addModule(std::move(libModule));

    //compileAndDeploy();

}

void Runtime::compileAndDeploy() {
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
    if (_isDeployed) jit.removeModule(_deployKey);
    _deployKey = jit.addModule(_module);
    _isDeployed = true;

    // update pointers for things that need to be accessed
    auto initFuncPtr = (void (*) ()) jit.getSymbolAddress(initFunc);
    _generateFuncPtr = (void (*) ()) jit.getSymbolAddress(generateFunc);

    //_outputPtr = (void*) jit.getSymbolAddress(_outputGlobal);
    _globalCtxPtr = (void*) jit.getSymbolAddress(ctxGlobal);

    // run the damn thing!
    initFuncPtr();

    _mainSchematic.updateCurrentPtr(_globalCtxPtr);
}

void Runtime::generate() {
    _generateFuncPtr();
}

llvm::GlobalVariable* Runtime::outputPtr(llvm::Module *module) {
    if (auto val = module->getGlobalVariable(outputName)) {
        return val;
    }

    return new llvm::GlobalVariable(*module, context()->numType()->get(), false, llvm::GlobalVariable::ExternalLinkage, nullptr, outputName);
}
