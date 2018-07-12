#pragma once

#include <string>

#include "Block.h"
#include "OwnedObject.h"
#include "RootRef.h"
#include "SurfaceRef.h"

namespace MaximCompiler {

    class Transaction : public OwnedObject {
    public:
        Transaction();

        RootRef buildRoot();

        SurfaceRef buildSurface(uint64_t id, const QString &name);

        void buildBlock(Block block);

        void printToStdout() const;
    };
}
