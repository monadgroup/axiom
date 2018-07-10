#pragma once

#include "OwnedObject.h"
#include "Transaction.h"

namespace MaximCompiler {

    class Jit;

    class Runtime : public OwnedObject {
    public:
        Runtime(bool includeUi, bool minSize, Jit *jit);

        uint64_t nextId();

        void runUpdate();

        void setBpm(float bpm);

        void setSampleRate(float sampleRate);

        void commit(Transaction transaction);

        bool isNodeExtracted(uint64_t surface, size_t node);

        void *getPortalPtr(size_t portal);

        void *getRootPtr();

        void *getNodePtr(uint64_t surface, void *surfacePtr, size_t node);

        void *getSurfacePtr(void *nodePtr);

        void *getBlockPtr(void *nodePtr);

        MaximFrontend::ControlPointers getControlPtrs(uint64_t block, void *blockPtr, size_t control);
    };
}
