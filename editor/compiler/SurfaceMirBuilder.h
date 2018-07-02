#pragma once

#include "interface/Runtime.h"
#include "interface/Transaction.h"
#include "../model/objects/NodeSurface.h"

namespace MaximCompiler {

    class SurfaceMirBuilder {
    public:

        static void build(Runtime &runtime, Transaction &transaction, AxiomModel::NodeSurface *surface);
    };

}
