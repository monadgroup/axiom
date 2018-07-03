#pragma once

#include <QHash>

#include "interface/Runtime.h"
#include "interface/Transaction.h"

namespace AxiomModel {
    class NodeSurface;
}

namespace MaximCompiler {

    struct ControlMirData {
        bool writtenTo;
        bool readFrom;
    };

    struct NodeMirData {
        QHash<QUuid, ControlMirData> controls;
    };

    class SurfaceMirBuilder {
    public:

        static void build(Runtime &runtime, Transaction &transaction, AxiomModel::NodeSurface *surface,
                          const QHash<QUuid, NodeMirData> &nodeData);
    };

}
