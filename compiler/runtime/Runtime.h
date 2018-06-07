#pragma once

#include <mutex>

#include "Jit.h"
#include "ValueOperator.h"
#include "RootSurface.h"
#include "../codegen/MaximContext.h"

namespace MaximCodegen {
    class MaximContext;
};

namespace MaximRuntime {

    struct QueuedEvent {
        int64_t deltaFrames;
        MidiEventValue event;
    };

    struct BpmVector {
        float a;
        float b;
    };

    class Runtime {
    public:
        static constexpr size_t eventQueueSize = 256;

        Runtime();

        MaximCodegen::MaximContext *ctx() { return &_context; }

        std::mutex &mutex() { return _mutex; }

        Jit &jit() { return _jit; }

        ValueOperator &op() { return _op; }

        llvm::Module &libModule() { return _libModule; }

        const llvm::Module &libModule() const { return _libModule; }

        RootSurface *mainSurface() { return _mainSurface.get(); }

        GeneratableModuleClass *compile();

        void generate();

        void queueEvent(const QueuedEvent &event);

        void clearEvents();

        void fillBuffer(float **buffer, size_t size);

        void fillPartialBuffer(float **buffer, size_t length, const MidiValue &event);

        float getBpm() const;

        void setBpm(float newVal);

    private:
        std::mutex _mutex;
        Jit _jit;
        MaximCodegen::MaximContext _context;
        ValueOperator _op;
        llvm::Module _libModule;
        std::unique_ptr<RootSurface> _mainSurface;

        QueuedEvent _queuedEvents[eventQueueSize];

        bool _isDeployed = false;
        Jit::ModuleKey _deployKey;

        BpmVector *_bpmVector = nullptr;
        void (*_generateFuncPtr)() = nullptr;

        llvm::Function *createForwardFunc(llvm::Module *module, std::string name, llvm::Value *ctx,
                                          MaximCodegen::ModuleClassMethod *method);
    };

}
