#pragma once

#include <QtWidgets/QGraphicsScene>

#include "common/TrackedObject.h"

namespace AxiomModel {
    class NodeSurface;

    class Node;

    class ConnectionWire;
}

namespace AxiomGui {

    class ModulePreviewCanvas : public QGraphicsScene, public AxiomCommon::TrackedObject {
        Q_OBJECT

    public:
        explicit ModulePreviewCanvas(AxiomModel::NodeSurface *surface);

    private slots:

        void addNode(AxiomModel::Node *node);

        void addWire(AxiomModel::ConnectionWire *wire);
    };
}
