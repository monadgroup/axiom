#pragma once

#include <QHash>

#include "interface/Runtime.h"
#include "interface/Transaction.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace MaximCompiler {

    class SurfaceMirBuilder {
    public:
        static void build(Transaction &transaction, AxiomModel::NodeSurface *surface);
    };
}
