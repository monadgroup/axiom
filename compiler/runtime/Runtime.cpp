#include "Runtime.h"

#include <iostream>

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
    _op.buildConverters(_jit);

    _bpmVector = (BpmVector *) _jit.getSymbolAddress(_context.beatsPerSecName());

    // this must go after `setLibModule` as compilation reads from there
    _mainSurface = std::make_unique<RootSurface>(this);
}

void Runtime::compile() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << "Starting compile..." << std::endl;

    _jit.flushRemoveQueue();

    if (!_mainSurface->needsGraphUpdate()) {
        return;
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
