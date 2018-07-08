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
    };
}
