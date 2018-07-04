#pragma once

#include <string>

#include "Block.h"
#include "OwnedObject.h"
#include "SurfaceRef.h"

namespace MaximCompiler {

    class Transaction : public OwnedObject {
    public:
        Transaction();

        SurfaceRef buildSurface(uint64_t id, const QString &name);

        void buildBlock(Block block);

        void printToStdout() const;
    };
}
