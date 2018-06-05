#include "Runtime.h"

#include <iostream>
#include <llvm/Linker/Linker.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/IR/Verifier.h>

#include "../codegen/Operator.h"
#include "../codegen/Converter.h"
#include "../codegen/Control.h"
#include "../codegen/Function.h"
#include "../util.h"

using namespace MaximRuntime;

static const std::string initFuncName = "maxim.init";
static const std::string generateFuncName = "maxim.generate";
static const std::string globalCtxName = "maxim.globalCtx";

Runtime::Runtime() : _context(_jit.dataLayout()), _op(&_context), _libModule("lib", _context.llvm()) {
    _libModule.setDataLayout(_jit.dataLayout());
    _context.setLibModule(&_libModule);
    _jit.addModule(_libModule);
    _op.buildConverters(_jit);

    _exportDefinitionTy = llvm::StructType::get(_context.llvm(), {
        _context.dataLayoutType(), // storage alloc size
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {_context.voidPointerType()}, false), 0), // constructor func
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {_context.voidPointerType()}, false), 0), // generate func
        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {_context.voidPointerType()}, false), 0)  // destructor func
    }, false);

    _exportInstrumentTy = llvm::StructType::get(_context.llvm(), {
        _context.voidPointerType(),
        llvm::PointerType::get(_exportDefinitionTy, 0)
    }, false);

    _bpmVector = (BpmVector *) _jit.getSymbolAddress(_context.beatsPerSecName());
    assert(_bpmVector);

    // this must go after `setLibModule` as compilation reads from there
    _mainSurface = std::make_unique<RootSurface>(this);
}

GeneratableModuleClass *Runtime::compile() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << "Starting compile..." << std::endl;

    _jit.flushRemoveQueue();

    if (!_mainSurface->needsGraphUpdate()) {
        return _mainSurface->compile();
    }

    auto startClock = std::clock();

    // tell every unit to save its current value, so we can restore it later
    _mainSurface->saveValue();

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

    _mainSurface->pullMethods();

    // update pointers for things that need to be accessed
    auto ctxGlobalPtr = (void *) _jit.getSymbolAddress(globalCtxName);
    auto initFuncPtr = (void (*)()) _jit.getSymbolAddress(initFuncName);
    _generateFuncPtr = (void (*)()) _jit.getSymbolAddress(generateFuncName);
    assert(ctxGlobalPtr && initFuncPtr && _generateFuncPtr);

    // propagate new pointers
    _mainSurface->updateCurrentPtr(ctxGlobalPtr);
    _mainSurface->restoreValue();

    // initialize it
    initFuncPtr();

    auto endClock = (std::clock() - startClock) / (double) (CLOCKS_PER_SEC / 1000);
    std::cout << "Finished compile in " << endClock << " ms" << std::endl;

    return mainClass;
}

void Runtime::generate() {
    if (_generateFuncPtr) _generateFuncPtr();
}

void Runtime::queueEvent(const QueuedEvent &event) {
    for (size_t i = 0; i < eventQueueSize; i++) {
        if (_queuedEvents[i].deltaFrames >= 0) continue;
        _queuedEvents[i] = event;
        break;
    }
}

void Runtime::clearEvents() {
    for (size_t i = 0; i < eventQueueSize; i++) {
        if (_queuedEvents[i].deltaFrames < 0) continue;
        _queuedEvents[i].deltaFrames = -1;
    }
}

void Runtime::fillBuffer(float **buffer, size_t size) {
    ssize_t remainingSamples = size;

    float *runningOutputs[2];
    runningOutputs[0] = buffer[0];
    runningOutputs[1] = buffer[1];

    while (remainingSamples > 0) {
        auto samplesToNextEvent = remainingSamples;

        MidiValue triggerEvents{};
        triggerEvents.active = true;

        for (size_t i = 0; i < eventQueueSize; i++) {
            auto &event = _queuedEvents[i];
            if (event.deltaFrames < 0) break;

            if (!event.deltaFrames) {
                triggerEvents.pushEvent(event.event);
            } else if (event.deltaFrames < samplesToNextEvent) {
                samplesToNextEvent = event.deltaFrames;
            }
        }

        fillPartialBuffer(runningOutputs, (size_t) samplesToNextEvent, triggerEvents);

        for (size_t i = 0; i < eventQueueSize; i++) {
            if (_queuedEvents[i].deltaFrames < 0) break;
            _queuedEvents[i].deltaFrames -= samplesToNextEvent;
        }

        runningOutputs[0] += samplesToNextEvent;
        runningOutputs[1] += samplesToNextEvent;
        remainingSamples -= samplesToNextEvent;
    }
}

void Runtime::fillPartialBuffer(float **buffer, size_t length, const MidiValue &event) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto inputPtr = _mainSurface->input->control()->group()->currentValuePtr();
    auto outputPtr = _mainSurface->output->control()->group()->currentValuePtr();
    assert(inputPtr && outputPtr);

    // write midi events for first sample
    _op.writeMidi(inputPtr, event);

    for (size_t i = 0; i < length; i++) {
        generate();
        auto outputNum = _op.readNum(outputPtr);
        buffer[0][i] = outputNum.left;
        buffer[1][i] = outputNum.right;

        // clear midi events
        _op.writeMidiCount(inputPtr, 0);
    }
}

float Runtime::getBpm() const {
    return _bpmVector->a;
}

void Runtime::setBpm(float newVal) {
    _bpmVector->a = newVal;
    _bpmVector->b = newVal;
}

std::unique_ptr<llvm::Module> Runtime::exportSurface(const std::string &globalName) {
    std::cout << "Starting export" << std::endl;

    auto result = std::make_unique<llvm::Module>("linked", _context.llvm());
    result->setDataLayout(_jit.dataLayout());
    llvm::Linker linker(*result);
    linker.linkInModule(llvm::CloneModule(&_libModule));

    _jit.linker = &linker;
    _mainSurface->scheduleChildUpdate();
    auto mainClass = compile();
    _jit.linker = nullptr;

    // ensure all globals in the module are internal, since we only need to expose a single global we'll define soon
    for (auto &global : result->global_values()) {
        if (global.getName().startswith("maxim")) {
            global.setLinkage(llvm::GlobalValue::InternalLinkage);
        }
    }

    auto runConstructor = createExportFunc(result.get(), "instrument.constructor", mainClass->constructor());
    auto runGenerate = createExportFunc(result.get(), "instrument.generate", mainClass->generate());
    auto runDestructor = createExportFunc(result.get(), "instrument.destructor", mainClass->destructor());

    new llvm::GlobalVariable(
        *result, _exportDefinitionTy, false, llvm::GlobalValue::LinkageTypes::ExternalLinkage,
        llvm::ConstantStruct::get(_exportDefinitionTy, {
            ctx()->sizeOf(mainClass->storageType()),
            runConstructor,
            runGenerate,
            runDestructor
        }), globalName
    );

    generateExportExternal(result.get());

    result->print(llvm::errs(), nullptr);

    // optimize the entire thing
    std::cout << "Running optimizer" << std::endl;
    llvm::legacy::PassManager mpm;
    llvm::legacy::FunctionPassManager fpm(result.get());

    fpm.add(llvm::createVerifierPass());
    fpm.add(llvm::createTargetTransformInfoWrapperPass(_jit.targetMachine()->getTargetIRAnalysis()));

    llvm::PassManagerBuilder builder;
    builder.OptLevel = 2;
    builder.SizeLevel = 3;
    builder.Inliner = llvm::createFunctionInliningPass(builder.OptLevel, builder.SizeLevel, false);
    builder.LoopVectorize = true;
    builder.SLPVectorize = true;
    _jit.targetMachine()->adjustPassManager(builder);
    builder.populateFunctionPassManager(fpm);
    builder.populateModulePassManager(mpm);

    fpm.doInitialization();
    for (auto &f : *result) {
        fpm.run(f);
    }
    mpm.run(*result);

    std::cout << "Done!" << std::endl;
    return std::move(result);
}

void Runtime::generateExportExternal(llvm::Module *module) {
    auto dataLayoutType = llvm::Type::getIntNTy(ctx()->llvm(), ctx()->dataLayout().getPointerSizeInBits(0));
    auto allocFunction = module->getFunction("malloc");
    if (!allocFunction) {
        allocFunction = llvm::Function::Create(
            llvm::FunctionType::get(ctx()->voidPointerType(), {dataLayoutType}, false),
            llvm::Function::ExternalLinkage, "malloc", module
        );
    }

    auto freeFunction = module->getFunction("free");
    if (!freeFunction) {
        freeFunction = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()->llvm()), {ctx()->voidPointerType()}, false),
            llvm::Function::ExternalLinkage, "free", module
        );
    }

    auto memSetFunc = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::memset, ctx()->dataLayoutType());

    {
        auto createInstrumentFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::PointerType::get(_exportInstrumentTy, 0), {llvm::PointerType::get(_exportDefinitionTy, 0)}, false),
            llvm::Function::LinkageTypes::ExternalLinkage, "axiom_create_instrument", module
        );
        auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", createInstrumentFunc);
        MaximCodegen::Builder b(block);

        auto allocSize = b.CreateLoad(b.CreateStructGEP(_exportDefinitionTy, createInstrumentFunc->arg_begin(), 0));

        // for simplicity, we can put the allocated buffer as the last element in the structure
        auto instrumentSize = ctx()->sizeOf(_exportInstrumentTy);
        auto resultSize = b.CreateAdd(allocSize, instrumentSize);

        auto resultPtr = b.CreatePointerCast(b.CreateCall(allocFunction, {resultSize}), llvm::PointerType::get(_exportInstrumentTy, 0));
        auto bytePtrTy = llvm::IntegerType::getInt8PtrTy(ctx()->llvm());
        auto dataPtr = b.CreateAdd(b.CreatePointerCast(resultPtr, bytePtrTy), b.CreateBitCast(instrumentSize, bytePtrTy));

        b.CreateStore(dataPtr, b.CreateStructGEP(_exportInstrumentTy, resultPtr, 0));
        b.CreateStore(createInstrumentFunc->arg_begin(), b.CreateStructGEP(_exportInstrumentTy, resultPtr, 1));

        // fill the allocated buffer with zeros
        b.CreateCall(memSetFunc, {dataPtr, ctx()->constInt(8, 0, false), allocSize, ctx()->constInt(1, 0, false)});

        // run the constructor function to initialize everything
        b.CreateCall(b.CreateLoad(b.CreateStructGEP(_exportDefinitionTy, createInstrumentFunc->arg_begin(), 1)), {dataPtr});
        b.CreateRet(resultPtr);
    }

    {
        auto generateInstrumentFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()->llvm()), {llvm::PointerType::get(_exportInstrumentTy, 0)}, false),
            llvm::Function::LinkageTypes::ExternalLinkage, "axiom_generate", module
        );
        auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", generateInstrumentFunc);
        MaximCodegen::Builder b(block);

        auto dataPtr = b.CreateStructGEP(_exportInstrumentTy, generateInstrumentFunc->arg_begin(), 0);
        auto definition = b.CreateLoad(b.CreateStructGEP(_exportInstrumentTy, generateInstrumentFunc->arg_begin(), 1));
        auto generateFunc = b.CreateStructGEP(_exportDefinitionTy, definition, 2);
        b.CreateCall(b.CreateLoad(generateFunc), {dataPtr});
        b.CreateRetVoid();
    }

    {
        auto destroyInstrumentFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()->llvm()), {llvm::PointerType::get(_exportInstrumentTy, 0)}, false),
            llvm::Function::LinkageTypes::ExternalLinkage, "axiom_destroy_instrument", module
        );
        auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", destroyInstrumentFunc);
        MaximCodegen::Builder b(block);

        b.CreateCall(freeFunction, {destroyInstrumentFunc->arg_begin()});
        b.CreateRetVoid();
    }
}

llvm::Function *Runtime::createForwardFunc(llvm::Module *module, std::string name, llvm::Value *ctx,
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

llvm::Function* Runtime::createExportFunc(llvm::Module *module, std::string name,
                                          MaximCodegen::ModuleClassMethod *method) {
    auto func = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(_context.llvm()), {_context.voidPointerType()}, false),
        llvm::Function::LinkageTypes::InternalLinkage, name, module
    );
    auto block = llvm::BasicBlock::Create(_context.llvm(), "entry", func);
    MaximCodegen::Builder b(block);
    method->call(b, {}, func->arg_begin(), module, "");
    b.CreateRetVoid();
    return func;
}
