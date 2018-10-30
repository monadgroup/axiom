#pragma once

#include <QtWidgets/QGraphicsScene>

#include "common/TrackedObject.h"

namespace AxiomModel {
    class NodeSurface;

    class Node;

    class ConnectionWire;
}

namespace MaximCompiler {
    class Runtime;
}

namespace AxiomGui {

    class ModulePreviewCanvas : public QGraphicsScene, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        ModulePreviewCanvas(AxiomModel::NodeSurface *surface, MaximCompiler::Runtime *runtime);

    private slots:

        void addNode(AxiomModel::Node *node);

        void addWire(AxiomModel::ConnectionWire *wire);

    private:
        MaximCompiler::Runtime *runtime;
    };
}
