#pragma once

#include "../GridSurface.h"

namespace AxiomModel {

    class Node;

    class NodeSurface : public GridSurface {
    Q_OBJECT

    public:
        Node *node;

        explicit NodeSurface(Node *node);

        static QPoint schematicToNodeSurface(QPoint p) { return p * 2; }
        static QSize schematicToNodeSurface(QSize s) { return s * 2; }
        static QPoint nodeSurfaceToSchematic(QPoint p) { return p / 2; }
        static QSize nodeSurfaceToSchematic(QSize s) { return s / 2; }

    private slots:

        void setSize(QSize size);
    };

}
