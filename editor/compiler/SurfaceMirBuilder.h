#pragma once

namespace MaximCompiler {
    class Runtime;
    class Transaction;
}

namespace AxiomModel {
    class NodeSurface;
}

namespace MaximCompiler {

    class SurfaceMirBuilder {
    public:
        static void build(Transaction *transaction, AxiomModel::NodeSurface *surface);
    };
}
