#pragma once

#include "OwnedObject.h"
#include "Transaction.h"
#include "editor/model/Value.h"

namespace MaximCompiler {

    class Runtime : public OwnedObject {
    public:
        Runtime(bool includeUi, bool minSize);

        uint64_t nextId();

        void runUpdate();

        void setBpm(float bpm);

        float getBpm();

        void setSampleRate(float sampleRate);

        float getSampleRate();

        uint64_t *getProfileTimesPtr();

        void commit(Transaction transaction);

        bool isNodeExtracted(uint64_t surface, size_t node);

        AxiomModel::NumValue convertNum(AxiomModel::FormType targetForm, AxiomModel::NumValue value);

        void *getPortalPtr(size_t portal);

        void *getRootPtr();

        void *getNodePtr(uint64_t surface, void *surfacePtr, size_t node);

        uint32_t *getExtractedBitmaskPtr(uint64_t surface, void *surfacePtr, size_t node);

        void *getSurfacePtr(void *nodePtr);

        void *getBlockPtr(void *nodePtr);

        MaximFrontend::ControlPointers getControlPtrs(uint64_t block, void *blockPtr, size_t control);
    };
}
