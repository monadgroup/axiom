#pragma once

#include "OwnedObject.h"
#include "Transaction.h"

namespace MaximCompiler {

    class Runtime : public OwnedObject {
    public:
        Runtime();

        uint64_t nextId();

        void commit(Transaction transaction);
    };

}
